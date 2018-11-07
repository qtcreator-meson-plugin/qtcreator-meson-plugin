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

MesonConfigurationDialog::MesonConfigurationDialog(const QJsonArray &json, QWidget *parent): QDialog (parent)
{
    QVBoxLayout *outer = new QVBoxLayout;
    setLayout(outer);

    QScrollArea *scroll = new QScrollArea;
    outer->addWidget(scroll, 1);

    QWidget *form_wrap = new QWidget;

    QFormLayout *form = new QFormLayout;
    form_wrap->setLayout(form);

    scroll->setWidget(form_wrap);
    scroll->setWidgetResizable(true);

    for (const QJsonValue &val: json) {
        QJsonObject obj = val.toObject();
        const QString name = obj.value("name").toString();
        const QString description = obj.value("description").toString();
        const QString type = obj.value("type").toString();

        initialValues.insert(name, obj);

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
            cb->setEditable(false);
            for(const QJsonValue ch: obj.value("choices").toArray()) {
                const QString choice = ch.toString();
                cb->addItem(choice, choice);
            }
            cb->setCurrentIndex(cb->findData(obj.value("value").toString()));
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

}
