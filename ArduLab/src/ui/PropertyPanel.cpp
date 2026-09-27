#include "ui/PropertyPanel.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>

namespace ardulab {

PropertyPanel::PropertyPanel(QWidget* parent) : QWidget(parent) {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(8, 8, 8, 8);
    outer->addWidget(new QLabel(tr("<b>Properti Komponen</b>")));

    auto* form = new QFormLayout();
    idLabel_ = new QLabel("-");
    idLabel_->setWordWrap(true);
    statusLabel_ = new QLabel("-");
    refEdit_ = new QLineEdit(this);
    refEdit_->setObjectName("prop-reference");
    valueEdit_ = new QLineEdit(this);
    valueEdit_->setObjectName("prop-value");
    posLabel_ = new QLabel("-");
    rotLabel_ = new QLabel("-");

    form->addRow(tr("ID definisi"), idLabel_);
    form->addRow(tr("Status"), statusLabel_);
    form->addRow(tr("Reference"), refEdit_);
    form->addRow(tr("Value"), valueEdit_);
    form->addRow(tr("Posisi (mm)"), posLabel_);
    form->addRow(tr("Rotasi"), rotLabel_);
    outer->addLayout(form);
    outer->addStretch(1);

    connect(refEdit_, &QLineEdit::editingFinished, this, &PropertyPanel::commit);
    connect(valueEdit_, &QLineEdit::editingFinished, this, &PropertyPanel::commit);

    clearInstance();
}

void PropertyPanel::showInstance(const ComponentInstance& inst) {
    hasInstance_ = true;
    currentId_ = inst.instanceId;
    idLabel_->setText(inst.definition.id);
    statusLabel_->setText(statusToString(inst.definition.status));
    refEdit_->setText(inst.reference);
    valueEdit_->setText(inst.value);
    origRef_ = inst.reference;
    origVal_ = inst.value;
    posLabel_->setText(QString("%1, %2")
        .arg(inst.position.x, 0, 'f', 2).arg(inst.position.y, 0, 'f', 2));
    rotLabel_->setText(QString("%1°").arg(inst.rotation));
    refEdit_->setEnabled(true);
    valueEdit_->setEnabled(true);
}

void PropertyPanel::clearInstance() {
    hasInstance_ = false;
    currentId_ = InstanceId();
    idLabel_->setText("-");
    statusLabel_->setText("-");
    refEdit_->clear(); valueEdit_->clear();
    posLabel_->setText("-"); rotLabel_->setText("-");
    refEdit_->setEnabled(false);
    valueEdit_->setEnabled(false);
}

void PropertyPanel::commit() {
    if (!hasInstance_) return;
    const QString ref = refEdit_->text();
    const QString val = valueEdit_->text();
    if (ref == origRef_ && val == origVal_) return;
    origRef_ = ref; origVal_ = val;
    emit propertyEdited(currentId_, ref, val);
}

} // namespace ardulab
