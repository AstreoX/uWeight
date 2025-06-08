#pragma once
#include "Core/BaseWidget.h"
#include <QDateTime>
#include <QFont>
#include <QColor>
#include <QPainter>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QTimer>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>

// AI模型数据结构
struct AIModelInfo {
    QString name;           // 模型名称
    QString provider;       // 提供商
    double score;          // 评分
    int rank;              // 排名
    QString category;      // 分类
    QString lastUpdated;   // 最后更新时间
    QString capability;    // 能力类型（推理、编程、多模态等）
    QString dataSource;    // 数据来源
    
    AIModelInfo() : score(0.0), rank(0) {}
    
    AIModelInfo(const QString& n, const QString& p, double s, int r, const QString& c = "", const QString& lu = "", const QString& cap = "", const QString& ds = "")
        : name(n), provider(p), score(s), rank(r), category(c), lastUpdated(lu), capability(cap), dataSource(ds) {}
};

class AIRankingWidget : public BaseWidget {
    Q_OBJECT

public:
    explicit AIRankingWidget(const WidgetConfig& config, QWidget* parent = nullptr);
    ~AIRankingWidget() = default;

    void updateContent() override;

protected:
    void drawContent(QPainter& painter) override;
    void applyConfig() override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onNetworkReplyFinished();
    void onDataRefreshTimer();

private:
    void setupDefaultConfig();
    void setupNetworkManager();
    void parseCustomSettings();
    void initializeDefaultData();
    void fetchAIRankingData();
    void parseRankingData(const QJsonDocument& doc);
    
    // 数据源API相关方法
    QString getApiUrlForDataSource() const;
    
    // 不同数据源的解析方法
    void parseHuggingFaceData(const QJsonDocument& doc);
    void parseOpenAIEvalsData(const QJsonDocument& doc);
    void parsePapersWithCodeData(const QJsonDocument& doc);
    void parseChatBotArenaData(const QJsonDocument& doc);
    void parseDefaultData(const QJsonDocument& doc);
    void initializeRecentModelData();
    
    void drawHeader(QPainter& painter);
    void drawRankingList(QPainter& painter);
    void drawModelItem(QPainter& painter, const AIModelInfo& model, const QRect& itemRect, bool isEven);
    void drawLoadingIndicator(QPainter& painter);
    void drawErrorMessage(QPainter& painter);
    QString formatScore(double score) const;
    QColor getRankColor(int rank) const;

private:
    // 网络管理
    QNetworkAccessManager* m_networkManager;
    QTimer* m_refreshTimer;
    
    // 数据存储
    QList<AIModelInfo> m_aiModels;
    QDateTime m_lastUpdateTime;
    bool m_isLoading;
    bool m_hasError;
    QString m_errorMessage;
    
    // 显示设置
    int m_maxDisplayCount;          // 最大显示数量
    bool m_showProvider;            // 显示提供商
    bool m_showScore;               // 显示评分
    bool m_showLastUpdate;          // 显示最后更新时间
    bool m_autoRefresh;             // 自动刷新
    int m_refreshInterval;          // 刷新间隔(分钟)
    
    // 数据源和能力设置
    QString m_currentDataSource;    // 当前数据源
    QString m_currentCapability;    // 当前能力指标
    QStringList m_availableDataSources;  // 可用数据源列表
    QStringList m_availableCapabilities; // 可用能力指标列表
    
    // 样式设置
    QFont m_headerFont;
    QFont m_modelFont;
    QFont m_scoreFont;
    QColor m_headerColor;
    QColor m_textColor;
    QColor m_backgroundColor;
    QColor m_alternateBackgroundColor;
    QColor m_borderColor;
    QColor m_loadingColor;
    
    // 布局设置
    int m_itemHeight;
    int m_headerHeight;
    int m_itemPadding;
    int m_rankColumnWidth;
    int m_scoreColumnWidth;
}; 