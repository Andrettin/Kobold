// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MASKEDMOUSEAREA_H
#define MASKEDMOUSEAREA_H

#include <QImage>
#include <QQuickItem>


class MaskedMouseArea : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(bool pressed READ isPressed NOTIFY pressedChanged)
    Q_PROPERTY(bool containsMouse READ containsMouse NOTIFY containsMouseChanged)
    Q_PROPERTY(QUrl maskSource READ maskSource WRITE setMaskSource NOTIFY maskSourceChanged)
    Q_PROPERTY(qreal alphaThreshold READ alphaThreshold WRITE setAlphaThreshold NOTIFY alphaThresholdChanged)
    //Kobold start
    Q_PROPERTY(QPointF mousePos READ mousePos NOTIFY mousePosChanged)
    //Kobold end
    QML_ELEMENT

public:
    MaskedMouseArea(QQuickItem *parent = nullptr);

    bool contains(const QPointF &point) const override;

    bool isPressed() const { return m_pressed; }
    bool containsMouse() const { return m_containsMouse; }

    QUrl maskSource() const { return m_maskSource; }
    void setMaskSource(const QUrl &source);

    qreal alphaThreshold() const { return m_alphaThreshold; }
    void setAlphaThreshold(qreal threshold);

    //Kobold start
    const QPointF &mousePos() const
    {
        return m_mousePos;
    }

    void setMousePos(const QPointF &pos)
    {
        if (pos != m_mousePos) {
            m_mousePos = pos;
            emit mousePosChanged();
        }
    }
    //Kobold end

signals:
    void pressed();
    void released();
    void clicked();
    void canceled();
    void pressedChanged();
    void maskSourceChanged();
    void containsMouseChanged();
    void alphaThresholdChanged();
    //Kobold start
    void mousePosChanged();
    //Kobold end

protected:
    void setPressed(bool pressed);
    void setContainsMouse(bool containsMouse);
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void hoverEnterEvent(QHoverEvent *event) override;
    void hoverLeaveEvent(QHoverEvent *event) override;
    void mouseUngrabEvent() override;

    //Kobold start
    void mouseMoveEvent(QMouseEvent *event) override
    {
        setMousePos(event->position());
    }

    void hoverMoveEvent(QHoverEvent *event) override
    {
        setMousePos(event->position());
    }
    //Kobold end

private:
    bool m_pressed;
    QUrl m_maskSource;
    QImage m_maskImage;
    QPointF m_pressPoint;
    qreal m_alphaThreshold;
    bool m_containsMouse;
    //Kobold start
    QPointF m_mousePos;
    //Kobold end
};

#endif
