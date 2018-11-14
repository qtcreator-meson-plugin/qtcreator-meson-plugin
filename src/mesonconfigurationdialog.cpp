#include "mesonconfigurationdialog.h"

#include <utils/qtcprocess.h>

#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QFormLayout>
#include <QIntValidator>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QDialogButtonBox>

namespace MesonProjectManager
{

MesonConfigurationDialog::MesonConfigurationDialog(const QJsonArray &json, const QString &projectName, QWidget *parent): QDialog (parent)
{
    setWindowTitle(tr("Options for %1").arg(projectName));

    QVBoxLayout *outer = new QVBoxLayout;
    setLayout(outer);

    QScrollArea *scroll = new QScrollArea;
    outer->addWidget(scroll, 1);

    QWidget *form_wrap = new QWidget;

    QFormLayout *form = new QFormLayout;
    form_wrap->setLayout(form);

    scroll->setWidget(form_wrap);
    scroll->setWidgetResizable(true);

    const QVector<QPair<QString, QStringList>> categories = {
        {"Core Options",     {"auto_features",
                              "backend",
                              "buildtype",
                              "debug",
                              "default_library",
                              "install_umask",
                              "layout",
                              "optimization",
                              "strip",
                              "unity",
                              "warning_level",
                              "werror",
                              "wrap_mode",}},
        {"Backend Options",  {"backend_max_links"}},
        {"Base Options",     {"b_asneeded",
                              "b_colorout",
                              "b_coverage",
                              "b_lto",
                              "b_lundef",
                              "b_ndebug",
                              "b_pch",
                              "b_pgo",
                              "b_pie",
                              "b_sanitize",
                              "b_staticpic",}},
        {"Compiler options", {"cpp_args",
                              "cpp_debugstl",
                              "cpp_link_args",
                              "cpp_std",
                              "c_args",
                              "c_link_args",
                              "c_std",}},
        {"Directories",      {"bindir",
                              "datadir",
                              "includedir",
                              "infodir",
                              "libdir",
                              "libexecdir",
                              "localedir",
                              "localstatedir",
                              "mandir",
                              "prefix",
                              "sbindir",
                              "sharedstatedir",
                              "sysconfdir",}},
        {"Testing options",  {"errorlogs",
                              "stdsplit",}},
    };

    for (const QJsonValue &val: json) {
        QJsonObject obj = val.toObject();
        const QString name = obj.value("name").toString();
        const QString description = obj.value("description").toString();
        const QString type = obj.value("type").toString();

        initialValues.insert(name, obj);
    }

    QSet<QString> placedOptions;

    for (const auto &categoryAndOptions: categories) {
        addHeading(form, categoryAndOptions.first);

        int added = 0;
        for (const QString &option: categoryAndOptions.second) {
            if (initialValues.contains(option)) {
                QJsonObject obj = initialValues.value(option);
                addControl(form, obj);
                placedOptions.insert(option);
                added++;
            }
        }
        if (added == 0) {
            form->removeRow(form->count()-1); // Kill the heading
        }
    }

    addHeading(form, tr("Project Options"));
    int userOptions = 0;
    for (const QJsonValue &val: json) {
        QJsonObject obj = val.toObject();
        if (!placedOptions.contains(obj.value("name").toString())) {
            addControl(form, obj);
            userOptions++;
        }
    }
    if (userOptions == 0) {
        form->removeRow(form->count()-1); // Kill the heading
    }

    QDialogButtonBox *dbb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(dbb, &QDialogButtonBox::accepted, this, [this] {
        for(const auto &transformer: transformers) {
            QJsonObject obj = transformer();
            const QString name = obj.value("name").toString();
            if(obj != initialValues.value(name)) {
                changedValues.insert(name, obj);
            }
        }

        QDialog::accept();
    });
    connect(dbb, &QDialogButtonBox::rejected, this, &QDialog::reject);
    outer->addWidget(dbb);

    resize(scroll->rect().width(), height());
}

QMap<QString, QJsonObject> MesonConfigurationDialog::getChangedValues() const
{
    return changedValues;
}

void MesonConfigurationDialog::addControl(QFormLayout *form, QJsonObject obj)
{
    const QString name = obj.value("name").toString();
    const QString description = obj.value("description").toString();
    const QString type = obj.value("type").toString();

    QLabel *lbl = new QLabel(QStringLiteral("%1 (%2)").arg(description, name));

    if (type == "array") {
        QStringList args;
        for(const QJsonValue val: obj.value("value").toArray()) {
            args.append(val.toString());
        }
        const QString shellquoted = Utils::QtcProcess::joinArgs(args, Utils::OsTypeLinux);
        QLineEdit *le = new QLineEdit(shellquoted);
        form->addRow(lbl, le);
        transformers.append([obj, le] {
            QJsonObject out = obj;
            out.insert("value", QJsonArray::fromStringList(Utils::QtcProcess::splitArgs(le->text(), Utils::OsTypeLinux)));
            return out;
        });
    } else if (type == "boolean") {
        QCheckBox *cb = new QCheckBox;
        cb->setChecked(obj.value("value").toBool());
        form->addRow(lbl, cb);
        transformers.append([obj, cb] {
            QJsonObject out = obj;
            out.insert("value", cb->isChecked());
            return out;
        });
    } else if (type == "combo") {
        QComboBox *cb = new QComboBox;
        const QString value = obj.value("value").toString();
        cb->setEditable(false);
        for(const QJsonValue ch: obj.value("choices").toArray()) {
            const QString choice = ch.toString();
            cb->addItem(choice, choice);
        }
        if (name=="backend" && value=="ninja") {
            cb->setEnabled(false);
        }
        cb->setCurrentIndex(cb->findData(value));
        form->addRow(lbl, cb);
        transformers.append([obj, cb] {
            QJsonObject out = obj;
            out.insert("value", cb->currentData().toString());
            return out;
        });
    } else if (type == "integer") {
        QLineEdit *le = new QLineEdit;
        le->setText(QString::number(obj.value("value").toInt()));
        int min = INT_MIN;
        int max = INT_MAX;
        if(obj.contains("min"))
            min = obj.value("min").toInt();
        if(obj.contains("max"))
            max = obj.value("max").toInt();
        le->setValidator(new QIntValidator(min, max));
        form->addRow(lbl, le);
        transformers.append([obj, le] {
            QJsonObject out = obj;
            out.insert("value", le->text().toInt());
            return out;
        });
    } else if (type == "string") {
        QLineEdit *le = new QLineEdit;
        le->setText(obj.value("value").toString());
        form->addRow(lbl, le);
        transformers.append([obj, le] {
            QJsonObject out = obj;
            out.insert("value", le->text());
            return out;
        });
    } else {
        form->addRow(lbl, new QLabel("Can't configure"));
    }
}

void MesonConfigurationDialog::addHeading(QFormLayout *form, const QString &name)
{
    QLabel *lbl = new QLabel(name);
    form->addRow(new QWidget); // Add a spacer
    lbl->setStyleSheet(QStringLiteral("font-weight: bold;"));
    form->setWidget(form->count(), QFormLayout::SpanningRole, lbl);
}

}
