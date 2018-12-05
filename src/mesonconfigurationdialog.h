#pragma once

#include <QDialog>
#include <QFormLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QVector>

namespace MesonProjectManager
{

class MesonConfigurationDialog: public QDialog
{
public:
    MesonConfigurationDialog(const QJsonArray &json, const QString &projectName, const QString &buildDir, const bool isNew, QWidget *parent=nullptr);
    QMap<QString, QJsonObject> getChangedValues() const;

private:
    void addControl(QFormLayout *form, QJsonObject obj);
    void addHeading(QFormLayout *form, const QString &name);
    void addText(QFormLayout *form, const QString &text);

    QMap<QString, QJsonObject> changedValues;
    QMap<QString, QJsonObject> initialValues;
    QVector<std::function<QJsonObject()>> transformers;
};

}
