!/bin/bash
# =========================================================
# Qt 微桌面系统 - 带循环重启
# =========================================================

APP_DIR="/usr/local/qt-app"
STATE_FILE="$APP_DIR/tmp/desktop_state"
DESKTOP_BIN="$APP_DIR/Desktop"
PID_FILE="$APP_DIR/tmp/desktop.pid"

# 防重入检查
if [ -f "$PID_FILE" ]; then
    OLD_PID=$(cat "$PID_FILE" 2>/dev/null)
    if [ -n "$OLD_PID" ] && kill -0 "$OLD_PID" 2>/dev/null; then
        echo "错误：另一个实例已在运行 (PID: $OLD_PID)"
        exit 1
    fi
fi
echo $$ > "$PID_FILE"

cleanup() {
    rm -f "$PID_FILE"
    exit 0
}
trap cleanup EXIT INT TERM

echo "========================================"
echo " Qt 微桌面系统启动 (PID: $$)"
echo "========================================"

# 环境设置
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

mkdir -p "$APP_DIR/tmp"

# =========================================================
# 主循环
# =========================================================
RESTART_COUNT=0
MAX_RESTART=10

while [ $RESTART_COUNT -lt $MAX_RESTART ]; do
    echo "desktop" > "$STATE_FILE"
    
    # 检查是否已有 Desktop 在运行
    if pgrep -x "Desktop" > /dev/null; then
        echo "警告：已有 Desktop 在运行，等待结束..."
        sleep 2
        pkill -9 Desktop 2>/dev/null
        sleep 1
    fi
    
    echo ""
    echo "==== 启动 Desktop (第 $((RESTART_COUNT+1)) 次) ===="
    RESTART_COUNT=$((RESTART_COUNT + 1))
    
    "$DESKTOP_BIN"
    DESKTOP_EXIT=$?
    
    echo "Desktop 退出，返回码: $DESKTOP_EXIT"
    
    # 读取状态
    state=$(cat "$STATE_FILE" 2>/dev/null || echo "desktop")
    echo "当前状态: $state"
    
    if [ "$state" = "app" ]; then
        echo "Desktop 切换到 App，等待 App 结束..."
        
        count=0
        while true; do
            sleep 1
            state=$(cat "$STATE_FILE" 2>/dev/null || echo "desktop")
            
            # ⭐ 关键：检测到状态变回 desktop 就跳出
            if [ "$state" != "app" ]; then
                echo "检测到状态变化: $state"
                break
            fi
            
            count=$((count + 1))
            if [ $count -gt 300 ]; then
                echo "App 超时，强制结束"
                echo "desktop" > "$STATE_FILE"
                break
            fi
            
            # 每秒打印一次，方便调试
            if [ $((count % 5)) -eq 0 ]; then
                echo "  等待中... ($count 秒)"
            fi
        done
        
        echo "App 已结束，返回 Desktop"
        RESTART_COUNT=0  # 正常切换，重置计数
        
    else
        echo "Desktop 异常退出，1 秒后重启..."
        sleep 1
    fi
done

echo "错误：重启次数过多"
cleanup

