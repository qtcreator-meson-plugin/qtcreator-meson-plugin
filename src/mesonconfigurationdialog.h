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
    MesonConfigurationDialog(const QJsonArray &json, const QString &projectName, QWidget *parent=nullptr);
    QMap<QString, QJsonObject> getChangedValues() const;

private:
    void addControl(QFormLayout *form, QJsonObject obj);
    void addHeading(QFormLayout *form, const QString &name);

    QMap<QString, QJsonObject> changedValues;
    QMap<QString, QJsonObject> initialValues;
    QVector<std::function<QJsonObject()>> transformers;
};

}
