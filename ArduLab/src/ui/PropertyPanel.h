#pragma once
#include "component/ComponentInstance.h"
#include <QWidget>

class QLineEdit;
class QLabel;

namespace ardulab {

// Right panel: shows/edits the selected instance's reference and value, and
// displays position + rotation (which are changed via canvas/toolbar).
class PropertyPanel : public QWidget {
    Q_OBJECT
public:
    explicit PropertyPanel(QWidget* parent = nullptr);

    void showInstance(const ComponentInstance& inst);
    void clearInstance();

signals:
    void propertyEdited(const InstanceId& id,
                        const QString& reference, const QString& value);

private:
    void commit();
    InstanceId currentId_;
    bool hasInstance_ = false;
    QLineEdit* refEdit_;
    QLineEdit* valueEdit_;
    QLabel* idLabel_;
    QLabel* posLabel_;
    QLabel* rotLabel_;
    QLabel* statusLabel_;
    QString origRef_, origVal_;
};

} // namespace ardulab
