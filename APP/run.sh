#!/bin/bash
# =========================================================
# App 启动器 - 负责 App 生命周期与 desktop_state 回写
# =========================================================

set -u

APP_DIR="/usr/local/qt-app"
TMP_DIR="$APP_DIR/tmp"
APP_NAME="$1"
STATE_FILE="$TMP_DIR/desktop_state"
STATE_TMP_FILE="$TMP_DIR/desktop_state.tmp"
LAUNCH_LOCK_FILE="$TMP_DIR/app_launcher.lock"
CURRENT_APP_FILE="$TMP_DIR/current_app"
APP_PID_FILE="$TMP_DIR/current_app.pid"
LOG_TAG="[app-run]"

mkdir -p "$TMP_DIR"

log() {
    echo "$(date '+%F %T') $LOG_TAG $*"
}

setup_runtime_env() {
    export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb0
    export QT_QPA_PLATFORM_PLUGIN_PATH=/usr/lib/plugins
    export QT_QPA_FONTDIR=/usr/lib/fonts
    chmod 666 /dev/fb0 2>/dev/null || true
    chmod 666 /dev/input/event2 2>/dev/null || true
    export TSLIB_TSDEVICE=/dev/input/event2
    export TSLIB_CONFFILE=/etc/ts.conf
    export TSLIB_CALIBFILE=/etc/pointercal
    export TSLIB_FBDEVICE=/dev/fb0
    export QT_QPA_GENERIC_PLUGINS=tslib
    export QT_QPA_FB_TSLIB=1
    export XDG_RUNTIME_DIR=/tmp/runtime-root
    mkdir -p "$XDG_RUNTIME_DIR"
    chown root:root "$XDG_RUNTIME_DIR" 2>/dev/null || true
    chmod 700 "$XDG_RUNTIME_DIR"
}

write_state() {
    printf '%s\n' "$1" > "$STATE_TMP_FILE" && mv -f "$STATE_TMP_FILE" "$STATE_FILE"
    log "desktop_state => $1"
}

APP_EXIT=0
APP_BIN=""

cleanup() {
    write_state "desktop"
    rm -f "$CURRENT_APP_FILE" "$APP_PID_FILE" "$STATE_TMP_FILE"
    log "App 生命周期结束，退出码: $APP_EXIT"
}

trap cleanup EXIT

exec 8>"$LAUNCH_LOCK_FILE"
if ! flock -n 8; then
    log "拒绝启动：已有另一个 App 启动流程正在执行"
    APP_EXIT=2
    exit 2
fi

log "请求启动 App: $APP_NAME"

if [ -z "$APP_NAME" ]; then
    log "错误：未指定 App 名称"
    APP_EXIT=1
    exit 1
fi

if [ -f "$APP_PID_FILE" ]; then
    RUNNING_APP_PID=$(cat "$APP_PID_FILE" 2>/dev/null)
    if [ -n "$RUNNING_APP_PID" ] && kill -0 "$RUNNING_APP_PID" 2>/dev/null; then
        log "拒绝启动：已有 App 正在运行 (PID: $RUNNING_APP_PID)"
        APP_EXIT=3
        exit 3
    fi
fi

setup_runtime_env

APP_PATH="$APP_DIR/app/$APP_NAME"

if [ -d "$APP_PATH" ]; then
    log "App 是目录，查找可执行文件: $APP_PATH"
    APP_BIN=$(find "$APP_PATH" -maxdepth 1 -type f -executable ! -name "run.sh" | head -1)
    if [ -z "$APP_BIN" ]; then
        log "错误：目录内未找到可执行文件"
        APP_EXIT=1
        exit 1
    fi
elif [ -f "$APP_PATH" ]; then
    APP_BIN="$APP_PATH"
else
    log "错误：App 不存在: $APP_PATH"
    APP_EXIT=1
    exit 1
fi

if [ ! -x "$APP_BIN" ]; then
    log "添加执行权限: $APP_BIN"
    chmod +x "$APP_BIN" 2>/dev/null || {
        log "错误：无法添加执行权限"
        APP_EXIT=1
        exit 1
    }
fi

write_state "app"
printf '%s\n' "$APP_NAME" > "$CURRENT_APP_FILE"
log "开始执行 App: name=$APP_NAME bin=$APP_BIN"
cd "$(dirname "$APP_BIN")" || {
    log "错误：无法进入 App 目录"
    APP_EXIT=1
    exit 1
}

"./$(basename "$APP_BIN")" &
APP_PID=$!
printf '%s\n' "$APP_PID" > "$APP_PID_FILE"
wait "$APP_PID"
APP_EXIT=$?
log "App 退出: name=$APP_NAME pid=$APP_PID exit_code=$APP_EXIT"
exit "$APP_EXIT"
