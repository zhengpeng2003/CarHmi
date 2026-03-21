#!/bin/bash
# =========================================================
# App 启动器 - 单次启动版（失败直接返回）
# =========================================================

APP_DIR="/usr/local/qt-app"
APP_NAME="$1"

echo "========================================"
echo " 启动 App: $APP_NAME"
echo "========================================"

# ---------------- 参数检查 ----------------
if [ -z "$APP_NAME" ]; then
    echo "错误：未指定 App 名称"
    echo "desktop" > "$APP_DIR/tmp/desktop_state"
    exit 1
fi

# ---------------- Qt / framebuffer 环境 ----------------
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb0
export QT_QPA_PLATFORM_PLUGIN_PATH=/usr/lib/plugins
export QT_QPA_FONTDIR=/usr/lib/fonts

chmod 666 /dev/fb0 2>/dev/null
chmod 666 /dev/input/event2 2>/dev/null

# ---------------- TSLIB 触摸 ----------------
export TSLIB_TSDEVICE=/dev/input/event2
export TSLIB_CONFFILE=/etc/ts.conf
export TSLIB_CALIBFILE=/etc/pointercal
export TSLIB_FBDEVICE=/dev/fb0
export QT_QPA_GENERIC_PLUGINS=tslib
export QT_QPA_FB_TSLIB=1

# ---------------- 运行时目录 ----------------
export XDG_RUNTIME_DIR=/tmp/runtime-root
mkdir -p "$XDG_RUNTIME_DIR"
chmod 700 "$XDG_RUNTIME_DIR"

# ---------------- App 路径检查 ----------------
APP_PATH="$APP_DIR/app/$APP_NAME"

# 如果是目录，找里面的可执行文件
if [ -d "$APP_PATH" ]; then
    echo "App 是目录，查找可执行文件..."
    APP_BIN=$(find "$APP_PATH" -maxdepth 1 -type f -executable ! -name "run.sh" | head -1)
    
    if [ -z "$APP_BIN" ]; then
        echo "错误：在 $APP_PATH 中未找到可执行文件"
        echo "desktop" > "$APP_DIR/tmp/desktop_state"
        exit 1
    fi
    echo "找到可执行文件: $APP_BIN"
elif [ -f "$APP_PATH" ]; then
    APP_BIN="$APP_PATH"
else
    echo "错误：App 不存在: $APP_PATH"
    echo "desktop" > "$APP_DIR/tmp/desktop_state"
    exit 1
fi

if [ ! -x "$APP_BIN" ]; then
    echo "添加执行权限..."
    chmod +x "$APP_BIN" 2>/dev/null || {
        echo "错误：无法添加执行权限"
        echo "desktop" > "$APP_DIR/tmp/desktop_state"
        exit 1
    }
fi

# ---------------- 启动 App ----------------
echo "执行: $APP_BIN"

# 切换到 App 目录运行
cd "$(dirname "$APP_BIN")" || exit 1

# ⭐ 关键修改：不要用 exec！普通执行，这样后面能恢复状态
"./$(basename "$APP_BIN")"
APP_EXIT=$?

echo "App 退出，返回码: $APP_EXIT"

# ---------------- 恢复状态（关键！）----------------
echo "恢复 Desktop 状态"
echo "desktop" > "$APP_DIR/tmp/desktop_state"

exit $APP_EXIT

