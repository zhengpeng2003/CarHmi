#!/bin/bash
# =========================================================
# Qt 微桌面系统 - desktop/app 状态驱动启动器
# =========================================================

set -u

APP_DIR="/usr/local/qt-app"
TMP_DIR="$APP_DIR/tmp"
STATE_FILE="$TMP_DIR/desktop_state"
STATE_TMP_FILE="$TMP_DIR/desktop_state.tmp"
DESKTOP_BIN="$APP_DIR/Desktop"
PID_FILE="$TMP_DIR/desktop.pid"
LOCK_FILE="$TMP_DIR/desktop.lock"
LOG_TAG="[root-run]"

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

read_state() {
    tr -d '[:space:]' < "$STATE_FILE" 2>/dev/null
}

write_state() {
    printf '%s\n' "$1" > "$STATE_TMP_FILE" && mv -f "$STATE_TMP_FILE" "$STATE_FILE"
    log "desktop_state => $1"
}

exec 9>"$LOCK_FILE"
if ! flock -n 9; then
    log "错误：另一个桌面启动器实例已在运行"
    exit 1
fi

if [ -f "$PID_FILE" ]; then
    OLD_PID=$(cat "$PID_FILE" 2>/dev/null)
    if [ -n "$OLD_PID" ] && kill -0 "$OLD_PID" 2>/dev/null; then
        log "错误：另一个实例已在运行 (PID: $OLD_PID)"
        exit 1
    fi
fi
printf '%s\n' "$BASHPID" > "$PID_FILE"

cleanup() {
    rm -f "$PID_FILE" "$STATE_TMP_FILE"
}
trap cleanup EXIT INT TERM

log "Qt 微桌面系统启动 (PID: $BASHPID)"
setup_runtime_env

RESTART_COUNT=0
MAX_RESTART=10

state=$(read_state)
if [ "$state" != "app" ] && [ "$state" != "desktop" ]; then
    write_state "desktop"
fi

while [ $RESTART_COUNT -lt $MAX_RESTART ]; do
    state=$(read_state)
    [ -z "$state" ] && state="desktop"
    log "主循环状态检查: state=$state restart_count=$RESTART_COUNT"

    if [ "$state" = "app" ]; then
        log "检测到 app 状态，等待 App 正常退出后再恢复 Desktop"
        wait_count=0
        while true; do
            sleep 1
            state=$(read_state)
            [ -z "$state" ] && state="desktop"
            if [ "$state" = "desktop" ]; then
                log "检测到 App 已退出，原因: state 切回 desktop"
                RESTART_COUNT=0
                break
            fi
            wait_count=$((wait_count + 1))
            if [ $((wait_count % 5)) -eq 0 ]; then
                log "等待 App 中... ${wait_count}s"
            fi
            if [ $wait_count -gt 300 ]; then
                log "等待 App 超时，强制恢复 desktop 状态"
                write_state "desktop"
                break
            fi
        done
        continue
    fi

    if pgrep -x "Desktop" >/dev/null; then
        log "警告：检测到残留 Desktop 进程，先等待结束"
        sleep 2
        pkill -9 Desktop 2>/dev/null || true
        sleep 1
    fi

    RESTART_COUNT=$((RESTART_COUNT + 1))
    log "启动 Desktop，第 ${RESTART_COUNT} 次"
    write_state "desktop"
    "$DESKTOP_BIN"
    DESKTOP_EXIT=$?

    state=$(read_state)
    [ -z "$state" ] && state="desktop"
    log "Desktop 退出，返回码=$DESKTOP_EXIT 当前状态=$state"

    if [ "$state" = "app" ]; then
        log "退出原因：桌面已切换到 App，进入等待流程"
        continue
    fi

    log "退出原因：Desktop 异常或主动退出，1 秒后尝试重启"
    sleep 1
done

log "错误：重启次数过多，停止启动器"
exit 1
