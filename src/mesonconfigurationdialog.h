#pragma once

#include <QDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QVector>

namespace MesonProjectManager
{

class MesonConfigurationDialog: public QDialog
{
public:
    MesonConfigurationDialog(const QJsonArray &json, QWidget *parent=nullptr);
    QMap<QString, QJsonObject> getChangedValues() const;

private:
    QMap<QString, QJsonObject> changedValues;
    QMap<QString, QJsonObject> initialValues;
    QVector<std::function<QJsonObject()>> transformers;
};

}
