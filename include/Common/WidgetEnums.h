#pragma once

// 背景图片缩放模式枚举
enum class BackgroundScaleMode {
    Stretch,                        // 拉伸填充
    KeepAspectRatio,               // 保持宽高比
    KeepAspectRatioByExpanding,    // 保持宽高比并裁剪
    Center,                        // 居中显示
    Tile                          // 平铺
};

// 日历样式枚举
enum class CalendarStyle {
    Modern,                        // 现代风格
    Classic,                       // 经典风格
    Minimal,                       // 极简风格
    Rounded                        // 圆角风格
};

// 周开始日枚举
enum class WeekStartDay {
    Sunday = 0,                    // 周日开始
    Monday = 1                     // 周一开始
}; 