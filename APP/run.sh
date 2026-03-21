#!/bin/bash
# =========================================================
# App 启动器 - 负责 App 生命周期与 desktop_state 回写
# =========================================================

APP_DIR="/usr/local/qt-app"
APP_NAME="$1"
STATE_FILE="$APP_DIR/tmp/desktop_state"
LOG_TAG="[app-run]"

log() {
    echo "$(date '+%F %T') $LOG_TAG $*"
}

write_state() {
    printf '%s\n' "$1" > "$STATE_FILE"
    log "desktop_state => $1"
}

APP_EXIT=0

cleanup() {
    write_state "desktop"
    log "App 生命周期结束，退出码: $APP_EXIT"
}

trap cleanup EXIT

log "请求启动 App: $APP_NAME"

if [ -z "$APP_NAME" ]; then
    log "错误：未指定 App 名称"
    APP_EXIT=1
    exit 1
fi

export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb0
export QT_QPA_PLATFORM_PLUGIN_PATH=/usr/lib/plugins
export QT_QPA_FONTDIR=/usr/lib/fonts
chmod 666 /dev/fb0 2>/dev/null
chmod 666 /dev/input/event2 2>/dev/null
export TSLIB_TSDEVICE=/dev/input/event2
export TSLIB_CONFFILE=/etc/ts.conf
export TSLIB_CALIBFILE=/etc/pointercal
export TSLIB_FBDEVICE=/dev/fb0
export QT_QPA_GENERIC_PLUGINS=tslib
export QT_QPA_FB_TSLIB=1
export XDG_RUNTIME_DIR=/tmp/runtime-root
mkdir -p "$XDG_RUNTIME_DIR"
chmod 700 "$XDG_RUNTIME_DIR"

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
log "开始执行 App: name=$APP_NAME bin=$APP_BIN"
cd "$(dirname "$APP_BIN")" || {
    log "错误：无法进入 App 目录"
    APP_EXIT=1
    exit 1
}

"./$(basename "$APP_BIN")"
APP_EXIT=$?
log "App 退出: name=$APP_NAME exit_code=$APP_EXIT"
exit "$APP_EXIT"

