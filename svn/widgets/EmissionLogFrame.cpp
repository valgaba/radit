#include "widgets/EmissionLogFrame.h"

#include "core/EmissionLog.h"
#include "widgets/button.h"
#include "widgets/label.h"

#include <QDateEdit>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QShowEvent>
#include <QStringList>
#include <QTableWidget>
#include <QVBoxLayout>

EmissionLogFrame::EmissionLogFrame(EmissionLog *log, QWidget *parent)
    : Frame(parent), m_log(log)
{
    setObjectName(QStringLiteral("EmissionLogFrame"));
    auto *root=new QVBoxLayout(this);
    root->setContentsMargins(1,1,1,1);
    root->setSpacing(2);

    auto *header=new Frame(this);
    header->setObjectName(QStringLiteral("framebarra"));
    header->setFixedHeight(26);
    auto *headerLayout=new QHBoxLayout(header);
    headerLayout->setContentsMargins(7,0,3,0);
    auto *title=new Label(header);
    title->setText(tr("Emission log"));
    headerLayout->addWidget(title,1);
    auto *closeButton=new Button(header);
    closeButton->setObjectName(QStringLiteral("RaditModuleCloseButton"));
    closeButton->SetIcon("Close-hover.svg");
    closeButton->setIconSize(QSize(15,15));
    closeButton->setFixedSize(15,15);
    closeButton->setToolTip(tr("Hide emission log"));
    headerLayout->addWidget(closeButton);
    connect(closeButton,&QPushButton::clicked,this,&QWidget::hide);
    root->addWidget(header);

    auto *filters=new QHBoxLayout;
    filters->setContentsMargins(5,3,5,3);
    filters->setSpacing(5);
    filters->addWidget(new QLabel(tr("From"),this));
    m_fromDate=new QDateEdit(QDate::currentDate().addDays(-6),this);
    m_fromDate->setCalendarPopup(true);
    m_fromDate->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    m_fromDate->setObjectName(QStringLiteral("EmissionLogFromDate"));
    filters->addWidget(m_fromDate);
    filters->addWidget(new QLabel(tr("To"),this));
    m_toDate=new QDateEdit(QDate::currentDate(),this);
    m_toDate->setCalendarPopup(true);
    m_toDate->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    m_toDate->setObjectName(QStringLiteral("EmissionLogToDate"));
    filters->addWidget(m_toDate);
    auto *applyButton=new QPushButton(tr("Apply"),this);
    applyButton->setObjectName(QStringLiteral("EmissionLogApplyButton"));
    connect(applyButton,&QPushButton::clicked,this,&EmissionLogFrame::refresh);
    filters->addWidget(applyButton);
    auto *exportButton=new QPushButton(tr("Export CSV"),this);
    exportButton->setObjectName(QStringLiteral("EmissionLogExportButton"));
    connect(exportButton,&QPushButton::clicked,this,&EmissionLogFrame::exportCsv);
    filters->addWidget(exportButton);
    root->addLayout(filters);
    m_search=new QLineEdit(this);
    m_search->setObjectName(QStringLiteral("EmissionLogSearch"));
    m_search->setPlaceholderText(tr("Filter title, file or source"));
    connect(m_search,&QLineEdit::textChanged,this,&EmissionLogFrame::refresh);
    auto *searchRow=new QHBoxLayout;
    searchRow->setContentsMargins(5,0,5,2);
    searchRow->addWidget(m_search);
    auto *clearButton=new QPushButton(tr("Clear history"),this);
    clearButton->setObjectName(QStringLiteral("EmissionLogClearButton"));
    connect(clearButton,&QPushButton::clicked,this,&EmissionLogFrame::clearHistory);
    searchRow->addWidget(clearButton);
    root->addLayout(searchRow);

    m_table=new QTableWidget(this);
    m_table->setObjectName(QStringLiteral("EmissionLogTable"));
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({tr("Start"),tr("End"),tr("Source"),
                                        tr("Title"),tr("File"),tr("Result"),tr("Details")});
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->hide();
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1,QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2,QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3,QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(4,QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(5,QHeaderView::ResizeToContents);
    root->addWidget(m_table,1);

    connect(m_log,&EmissionLog::recordsChanged,this,[this]() {
        if (isVisible())
            refresh();
    });
    refresh();
}

void EmissionLogFrame::showEvent(QShowEvent *event)
{
    Frame::showEvent(event);
    refresh();
}

void EmissionLogFrame::refresh()
{
    if (!m_log || !m_table)
        return;
    m_table->setRowCount(0);
    QVector<EmissionRecord> records=m_log->recordsBetween(m_fromDate->date(),m_toDate->date());
    const QString needle=m_search->text().trimmed();
    for (auto it=records.crbegin();it!=records.crend();++it) {
        const EmissionRecord &record=*it;
        if (!needle.isEmpty()
            && !record.title.contains(needle,Qt::CaseInsensitive)
            && !record.path.contains(needle,Qt::CaseInsensitive)
            && !record.source.contains(needle,Qt::CaseInsensitive)
            && !record.result.contains(needle,Qt::CaseInsensitive))
            continue;
        const int row=m_table->rowCount();
        m_table->insertRow(row);
        const QStringList values={record.startedAt.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")),
            record.finishedAt.isValid()
                ? record.finishedAt.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")) : QString(),
            record.source,record.title,QFileInfo(record.path).fileName(),record.result,record.details};
        for (int column=0;column<values.size();++column) {
            auto *item=new QTableWidgetItem(values[column]);
            if (column==4)
                item->setToolTip(record.path);
            m_table->setItem(row,column,item);
        }
    }
}

void EmissionLogFrame::exportCsv()
{
    if (!m_log)
        return;
    const QString suggested=QStringLiteral("radit-emission-%1.csv")
                                .arg(QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd")));
    const QString filename=QFileDialog::getSaveFileName(this,tr("Export emission log"),suggested,
                                                          tr("CSV files (*.csv)"));
    if (filename.isEmpty())
        return;
    QString error;
    if (!m_log->exportCsv(filename,m_fromDate->date(),m_toDate->date(),m_search->text(),&error))
        QMessageBox::warning(this,tr("Export emission log"),
                             tr("Could not export the log: %1").arg(error));
}

void EmissionLogFrame::clearHistory()
{
    if (!m_log)
        return;
    const QMessageBox::StandardButton answer=QMessageBox::question(
        this,tr("Clear emission history"),
        tr("Are you sure you want to delete the entire emission history? This cannot be undone."),
        QMessageBox::Yes | QMessageBox::No,QMessageBox::No);
    if (answer!=QMessageBox::Yes)
        return;

    QString error;
    if (!m_log->clearHistory(&error))
        QMessageBox::warning(this,tr("Clear emission history"),
                             tr("Could not clear the entire emission history: %1").arg(error));
}
