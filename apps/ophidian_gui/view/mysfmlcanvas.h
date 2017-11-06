#ifndef MYSFMLCANVAS_H
#define MYSFMLCANVAS_H

#include <QKeyEvent>
#include <QWheelEvent>
#include <QResizeEvent>

#include <omp.h>
#include <memory>

#include <ophidian/geometry/Models.h>
#include "qsfmlcanvas.h"
#include "canvas.h"
#include "control/maincontroller.h"
#include "control/state.h"

using namespace qsfml;

class State;

class MySFMLCanvas : public QSFMLCanvas
{
public:
    MySFMLCanvas(QWidget *parent = 0);
    virtual ~MySFMLCanvas();

    void setController(MainController & controller);

    Canvas * canvas();

    void OnInit();
    void OnUpdate();
    void wheelEvent(QWheelEvent * e);
    void resizeEvent(QResizeEvent * e);
    void keyPressEvent(QKeyEvent * e);
    void mousePressEvent(QMouseEvent * e);
    void mouseMoveEvent(QMouseEvent * e);
    void mouseReleaseEvent(QMouseEvent * e);

    void centerViewOn(const ophidian::geometry::Point & p1);
    void viewSize(const ophidian::geometry::Point & size);

public slots:
    void setState(State * state);
    void setSize(ophidian::geometry::Point size);
    void reserveMinimumOfQuads(std::size_t minimumOfQuads);
    void updatePositionQuad(const ophidian::geometry::Point & p);

private:
    ophidian::geometry::Point mouseEventToPoint(QMouseEvent * e);

    MainController * mMainController;
    State * mState;
    Canvas mCanvas;
    sf::View mCameraView;
};

#endif // MYSFMLCANVAS_H
