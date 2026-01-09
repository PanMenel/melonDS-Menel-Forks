#include "RASettingsDialog.h"
#include "ui_RASettingsDialog.h"

#include "Config.h"
#include "main.h"
#include "RetroAchievements/RAClient.h"

#include <QMessageBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QHash>
#include <QPixmap>

RAContext* raContext = nullptr;
bool RASettingsDialog::needsReset = false;
RASettingsDialog::RASettingsDialog(EmuInstance* inst, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::RASettingsDialog)
    , emuInstance(inst)
{
    raContext = emuInstance->getRA();
    ui->setupUi(this);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &RASettingsDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &RASettingsDialog::reject);

    setAttribute(Qt::WA_DeleteOnClose);

    auto& instcfg = emuInstance->getLocalConfig();
    ui->cbRAEnabled->setChecked(instcfg.GetBool("RetroAchievements.Enabled"));
    ui->cbRAHardcore->setChecked(instcfg.GetBool("RetroAchievements.HardcoreMode"));
    ui->leRAUsername->setText(QString::fromStdString(instcfg.GetString("RetroAchievements.Username")));
    ui->leRAPassword->setText(QString::fromStdString(instcfg.GetString("RetroAchievements.Password")));

    auto UpdateRAUI = [this]() {
        RAContext* ra = emuInstance->getRA();
        if (!ra) return;

        bool loggedIn = ra->IsLoggedIn();
        bool emuRunning = emuInstance->emuIsActive();

        ui->cbRAEnabled->setEnabled(!loggedIn);
        ui->cbRAHardcore->setEnabled(!loggedIn);
        ui->leRAUsername->setEnabled(!loggedIn);
        ui->leRAPassword->setEnabled(!loggedIn);
        ui->btnRALogin->setEnabled(!emuRunning);

        if (emuRunning) {
            ui->btnRALogin->setToolTip(
                "Stop the emulation before logging in or out of RetroAchievements."
            );
        } else {
            ui->btnRALogin->setToolTip(QString());
        }

        if (loggedIn) {
            ui->btnRALogin->setText("Logout");

            const rc_client_user_t* user = rc_client_get_user_info(ra->client);
            if (user && user->display_name)
                ui->lblRAStatus->setText(QString("Logged in as: %1").arg(user->display_name));
            else
                ui->lblRAStatus->setText("Logged in");

            ui->lblRAStatus->setStyleSheet("font-weight: bold; color: #2ecc71;");
        } else {
            ui->btnRALogin->setText("Login Now");
            ui->lblRAStatus->setText("Not logged in");
            ui->lblRAStatus->setStyleSheet("color: gray;");
        }
    };

    UpdateRAUI();

    connect(ui->btnRALogin, &QPushButton::clicked, this, [this, UpdateRAUI]() {
        RAContext* ra = emuInstance->getRA();
        if (!ra) return;

        if (ra->IsLoggedIn()) {
            ra->SetLoggedIn(false);
            ra->SetToken("");
            UpdateRAUI();

            ui->cbRAEnabled->setProperty("user_originalValue", !ui->cbRAEnabled->isChecked());
            ui->cbRAHardcore->setProperty("user_originalValue", !ui->cbRAHardcore->isChecked());
            ui->leRAUsername->setProperty("user_originalValue", "");
            ui->leRAPassword->setProperty("user_originalValue", "");

            if (emuInstance && emuInstance->emuIsActive()) {
                done(QDialog::Accepted);
            }

        } else {
            std::string user = ui->leRAUsername->text().toStdString();
            std::string pass = ui->leRAPassword->text().toStdString();
            if (user.empty() || pass.empty()) return;

            ra->LoginWithPassword(user.c_str(), pass.c_str(), ui->cbRAHardcore->isChecked());

            ui->cbRAEnabled->setProperty("user_originalValue", !ui->cbRAEnabled->isChecked());
            ui->cbRAHardcore->setProperty("user_originalValue", !ui->cbRAHardcore->isChecked());
            ui->leRAUsername->setProperty("user_originalValue", "");
            ui->leRAPassword->setProperty("user_originalValue", "");

            if (emuInstance && emuInstance->emuIsActive()) {
                done(QDialog::Accepted);
            }
        }
    });

    RAContext* raTmp = emuInstance->getRA();
    if (raTmp) {
        auto mainWin = emuInstance->getMainWindow();
        raTmp->onLoginResponse = [this, UpdateRAUI, mainWin](bool success, const std::string& msg) {
            QMetaObject::invokeMethod(this, [this, UpdateRAUI, mainWin, success, msg]() {
                UpdateRAUI();
                if (mainWin) {
                    mainWin->ShowRALoginToast(success, msg);
                }
            });
        };
    }

    RAContext* raUI = emuInstance->getRA();
    if (raUI)
    {
    raUI->SetOnMeasuredProgress(
        [this](unsigned, unsigned, unsigned, const char*)
        {
            QMetaObject::invokeMethod(this, [this]() {
                UpdateAchievementsTab();
            });
        }
    );
    }

#define SET_ORIGVAL(type, val) \
    for (type* w : findChildren<type*>(nullptr)) \
        w->setProperty("user_originalValue", w->val());

    SET_ORIGVAL(QLineEdit, text);
    SET_ORIGVAL(QCheckBox, isChecked);

#undef SET_ORIGVAL
netManager = new QNetworkAccessManager(this);
UpdateAchievementsTab();
}

RASettingsDialog::~RASettingsDialog()
{
    if (raContext) {
        raContext->SetOnMeasuredProgress(nullptr);
        raContext->onLoginResponse = nullptr;
    }
    delete ui;
}

void RASettingsDialog::done(int r)
{
    if (!emuInstance)
    {
        QDialog::done(r);
        return;
    }

    needsReset = false;

#ifdef RETROACHIEVEMENTS_ENABLED
    if (r == QDialog::Accepted)
    {
        bool modified = false;

#define CHECK_ORIGVAL(type, val) \
        if (!modified) for (type* w : findChildren<type*>(nullptr)) \
        { \
            QVariant v = w->val(); \
            if (v != w->property("user_originalValue")) \
            { \
                modified = true; \
                break; \
            } \
        }

        CHECK_ORIGVAL(QLineEdit, text);
        CHECK_ORIGVAL(QCheckBox, isChecked);

#undef CHECK_ORIGVAL

        if (modified)
        {
            if (emuInstance->emuIsActive()
                && QMessageBox::warning(
                    this,
                    "Reset necessary to apply changes",
                    "The emulation will be reset for the changes to take place.",
                    QMessageBox::Ok,
                    QMessageBox::Cancel) != QMessageBox::Ok)
            {
                return;
            }

            auto& instcfg = emuInstance->getLocalConfig();

            instcfg.SetBool("RetroAchievements.Enabled", ui->cbRAEnabled->isChecked());
            instcfg.SetBool("RetroAchievements.HardcoreMode", ui->cbRAHardcore->isChecked());
            instcfg.SetString("RetroAchievements.Username", ui->leRAUsername->text().toStdString());
            instcfg.SetString("RetroAchievements.Password", ui->leRAPassword->text().toStdString());

            emuInstance->SyncRetroAchievementsFromConfig();
            Config::Save();

            needsReset = true;
        }
    }
#endif
    QDialog::done(r);
}

void RASettingsDialog::UpdateAchievementsTab()
{
    RAContext* ra = emuInstance->getRA();
    if (!ra || !ra->IsGameLoaded())
    {
        ui->lblRAAchievementsHeader->setText("No game loaded");
        ui->tblRAAchievements->setRowCount(0);
        return;
    }

    const auto& achievements = ra->GetAllAchievements();

    ui->lblRAAchievementsHeader->setText(
        QString("Achievements (%1)").arg(achievements.size())
    );

    ui->tblRAAchievements->setRowCount((int)achievements.size());

    ui->tblRAAchievements->setSortingEnabled(false);

    int row = 0;
    for (const auto& ach : achievements)
    {
        ui->tblRAAchievements->setItem(row, 0, new QTableWidgetItem());

        const char* badgeUrl =
            ach.unlocked ? ach.badge_url : ach.badge_locked_url;

        SetBadgeImage(row, badgeUrl);
        auto* title = new QTableWidgetItem(QString::fromStdString(ach.title));
        title->setToolTip(QString("ID: %1\nMeasured: %2")
            .arg(ach.id)
            .arg(ach.measured ? "Yes" : "No"));

        auto* desc = new QTableWidgetItem(QString::fromStdString(ach.description));
        
        auto* pts = new QTableWidgetItem(QString::number(ach.points));
        pts->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem* typeItem = new QTableWidgetItem();
        switch (ach.bucket_type)
        {
            case RC_CLIENT_ACHIEVEMENT_BUCKET_ACTIVE_CHALLENGE:
                typeItem->setText("Active Challenge");
                typeItem->setForeground(QColor("#e67e22"));
                break;
            case RC_CLIENT_ACHIEVEMENT_BUCKET_RECENTLY_UNLOCKED:
                typeItem->setText("Recently Unlocked");
                typeItem->setForeground(QColor("#9b59b6"));
                break;
            case RC_CLIENT_ACHIEVEMENT_BUCKET_ALMOST_THERE:
                typeItem->setText("Almost There");
                typeItem->setForeground(QColor("#3498db"));
                break;
            case RC_CLIENT_ACHIEVEMENT_BUCKET_LOCKED:
                typeItem->setText("Locked");
                typeItem->setForeground(QColor("#7f8c8d"));
                break;
            case RC_CLIENT_ACHIEVEMENT_BUCKET_UNOFFICIAL:
                typeItem->setText("Unofficial");
                typeItem->setForeground(QColor("#95a5a6"));
                break;
            case RC_CLIENT_ACHIEVEMENT_BUCKET_UNSUPPORTED:
                typeItem->setText("Unsupported");
                typeItem->setForeground(QColor("#c0392b"));
                break;
            case RC_CLIENT_ACHIEVEMENT_BUCKET_UNLOCKED:
                typeItem->setText("Unlocked");
                typeItem->setForeground(QColor("#2ecc71"));
                break;
            default:
                typeItem->setText("Unknown");
                typeItem->setForeground(QColor("#7f8c8d"));
                break;
        }
        typeItem->setTextAlignment(Qt::AlignCenter);
        ui->tblRAAchievements->setItem(row, 4, typeItem);

        ui->tblRAAchievements->removeCellWidget(row, 5);

        if (ach.measured && ach.target > 0) {
            QProgressBar* bar = new QProgressBar();
            bar->setMinimum(0);
            bar->setMaximum(ach.target);
            bar->setValue(ach.value);

            if (!ach.progressText.empty()) {
                bar->setFormat(QString("%1 (%2%)")
                    .arg(QString::fromStdString(ach.progressText))
                    .arg((ach.target > 0) ? (ach.value * 100 / ach.target) : 0));
            } else {
                bar->setFormat("%v/%m");
            }

            bar->setTextVisible(true);
            bar->setAlignment(Qt::AlignCenter);

            bar->setStyleSheet(
                "QProgressBar { border: 1px solid #999; border-radius: 3px; text-align: center; background: #fff; color: #000; }"
                "QProgressBar::chunk { background-color: #3498db; width: 10px; }"
            );

            ui->tblRAAchievements->setItem(row, 5, new QTableWidgetItem(""));
            ui->tblRAAchievements->setCellWidget(row, 5, bar);
        } else if (ach.measured) {
            auto* bar = new QProgressBar();
            bar->setMinimum(0);
            bar->setMaximum(1);
            bar->setValue(0);
            bar->setFormat("0");
            bar->setTextVisible(true);
            bar->setAlignment(Qt::AlignCenter);
            ui->tblRAAchievements->setItem(row, 5, new QTableWidgetItem(""));
            ui->tblRAAchievements->setCellWidget(row, 5, bar);
        } else {
            ui->tblRAAchievements->setItem(row, 5, new QTableWidgetItem(""));
        }

        ui->tblRAAchievements->setItem(row, 1, title);
        ui->tblRAAchievements->setItem(row, 2, desc);
        ui->tblRAAchievements->setItem(row, 3, pts);

        row++;
    }

    ui->tblRAAchievements->resizeRowsToContents();
    ui->tblRAAchievements->setSortingEnabled(true);
}
void RASettingsDialog::SetBadgeImage(int row, const char* url)
{
    if (!url || !*url)
        return;

    QString qurl = QString::fromUtf8(url);

    if (badgeCache.contains(qurl))
    {
        QLabel* lbl = new QLabel();
        lbl->setPixmap(badgeCache[qurl].scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        lbl->setAlignment(Qt::AlignCenter);
        ui->tblRAAchievements->setCellWidget(row, 0, lbl);
        return;
    }

    QNetworkReply* reply = netManager->get(QNetworkRequest(QUrl(qurl)));

    connect(reply, &QNetworkReply::finished, this, [this, reply, row, qurl]()
    {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError)
            return;

        QPixmap pix;
        if (!pix.loadFromData(reply->readAll()))
            return;

        badgeCache.insert(qurl, pix);

        QLabel* lbl = new QLabel();
        lbl->setPixmap(pix.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        lbl->setAlignment(Qt::AlignCenter);

        ui->tblRAAchievements->setCellWidget(row, 0, lbl);
    });
}