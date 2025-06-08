# Desktop Widget System

一个基于Qt6和C++的现代化桌面小部件系统，为Windows用户提供美观实用的桌面小部件功能。

## 项目概述

**Desktop Widget System** 是一个功能强大的桌面小部件管理系统，支持多种类型的小部件，包括时钟、天气、系统监控、日历、便签等。

### 主要特性

- 🎨 **多主题支持** - 内置多种精美主题，支持自定义主题
- 🔧 **灵活配置** - 每个小部件都可以独立配置和定制
- 📱 **系统托盘集成** - 完整的系统托盘支持，包括右键菜单
- 🎯 **多种小部件** - 时钟、天气、系统性能、日历、便签、AI排行榜等
- 💾 **配置持久化** - 自动保存小部件位置和配置
- 🚀 **高性能** - 基于Qt6框架，原生C++性能

## 技术栈

- **框架**: Qt 6.9.0+
- **语言**: C++17
- **构建**: CMake 3.20+
- **平台**: Windows 10/11
- **编译器**: MinGW-w64 / MSVC 2019+

## 快速开始

### 环境要求

- Windows 10/11
- Qt 6.9.0 或更高版本
- CMake 3.20+
- MinGW-w64 或 Visual Studio 2019+

### 编译构建

```bash
# 1. 克隆项目
git clone https://github.com/AstreoX/uWeight.git
cd uWeight

# 2. 创建构建目录
mkdir build
cd build

# 3. 配置CMake（请根据实际Qt安装路径修改）
cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.9.0/mingw_64"

# 4. 编译项目
cmake --build . --config Release

# 5. 运行程序
./bin/DesktopWidgetSystem.exe
```

## 功能展示

### 支持的小部件类型

- **🕐 时钟小部件** - 多种时间格式显示
- **🌤️ 天气小部件** - 实时天气信息
- **📊 系统性能监控** - CPU、内存、网络监控
- **📅 日历小部件** - 日期显示和提醒
- **📝 便签小部件** - 桌面便签功能
- **🤖 AI排行榜** - AI相关排行信息展示

### 主题系统

系统内置多种主题：
- **Default** - 默认清新主题
- **Dark** - 深色主题
- **Minimal** - 极简主题
- **Nature** - 自然主题
- **City** - 城市主题
- **Space** - 太空主题
- **Gradient** - 渐变主题

## 项目结构

```
Desktop Widget System/
├── src/                    # 源代码目录
│   ├── main.cpp           # 主程序入口
│   ├── Core/              # 核心组件
│   ├── Framework/         # 框架层
│   ├── Widgets/           # 小部件实现
│   ├── BackendManagement/ # 后端管理
│   └── Utils/             # 工具类
├── include/               # 头文件目录
├── theme_source/          # 主题资源
├── icons/                 # 图标资源
├── CMakeLists.txt        # CMake配置
└── README.md             # 项目说明
```

## 使用说明

1. **启动程序** - 运行DesktopWidgetSystem.exe
2. **系统托盘** - 程序会最小化到系统托盘
3. **创建小部件** - 右键托盘图标选择"创建小部件"
4. **配置小部件** - 右键小部件进行个性化配置
5. **主题切换** - 在管理界面中切换不同主题

## 开发指南

详细的技术文档请参阅 [技术说明文档.md](技术说明文档.md)

### 创建自定义小部件

```cpp
class CustomWidget : public BaseWidget {
    Q_OBJECT
    
public:
    explicit CustomWidget(const WidgetConfig& config, QWidget* parent = nullptr);
    
    void updateData() override;
    void saveConfiguration() override;
    void loadConfiguration() override;
};
```

## 贡献

欢迎提交Issue和Pull Request来帮助改进项目！

## 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件

## 联系方式

- **GitHub**: [AstreoX](https://github.com/AstreoX)
- **Email**: Gskyer.sky@Gmail.com
- **Issues**: [项目Issues页面](https://github.com/AstreoX/uWeight/issues)

---

⭐ 如果这个项目对您有帮助，请考虑给个星标！ 