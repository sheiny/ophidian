#include "mysfmlcanvas.h"

MySFMLCanvas::MySFMLCanvas(QWidget *parent) :
    QSFMLCanvas(parent),
    mCameraView(sf::FloatRect(0, 0, 51599.25, 34200.0))
{
    mCameraView.setViewport(sf::FloatRect(0.0, 0.0, 1.0, 1.0));
}

void MySFMLCanvas::setController(MainController & controller)
{
    mMainController = &controller;
}

void MySFMLCanvas::resizeEvent(QResizeEvent *e)
{
    sf::RenderWindow::create((sf::WindowHandle) winId());
}

MySFMLCanvas::~MySFMLCanvas()
{
}

Canvas * MySFMLCanvas::canvas()
{
    return &mCanvas;
}


void MySFMLCanvas::OnInit()
{
}


void MySFMLCanvas::OnUpdate()
{
    // ... mCanvas.update();

    //static boost::posix_time::ptime last;
    //boost::posix_time::ptime current = boost::posix_time::microsec_clock::local_time();
    //boost::posix_time::time_duration diff = current - last;

    clear(sf::Color::Black);
    setView(mCameraView);

    draw(mCanvas);

    setView(getDefaultView());
    display();

    /*std::string stdstring{std::to_string(1000./diff.total_milliseconds())};
    stdstring = "FPS: " + stdstring;
    sf::Text fps_text;
    fps_text.setString(sf::String(stdstring));
    fps_text.setPosition(30, 30);
    fps_text.setColor(sf::Color::Red);
    fps_text.setFont(m_opensans);
    draw(fps_text);
    last = current;*/
}

void MySFMLCanvas::wheelEvent(QWheelEvent *e)
{
    if(e->delta() > 0)
        mCameraView.zoom(1.f/1.1f);
    else if(e->delta() < 0)
        mCameraView.zoom(1.1f);
}
void MySFMLCanvas::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
    case Qt::Key::Key_W:
    case Qt::Key::Key_Up:
        mCameraView.move(sf::Vector2f(0.0f, mCameraView.getSize().y*.1));
        break;
    case Qt::Key::Key_S:
    case Qt::Key::Key_Down:
        mCameraView.move(sf::Vector2f(0.0f, -mCameraView.getSize().y*.1));
        break;
    case Qt::Key::Key_D:
    case Qt::Key::Key_Right:
        mCameraView.move(sf::Vector2f(-mCameraView.getSize().x*.1, 0.0f));
        break;
    case Qt::Key::Key_A:
    case Qt::Key::Key_Left:
        mCameraView.move(sf::Vector2f(mCameraView.getSize().x*.1, 0.0f));
        break;
    }
}

void MySFMLCanvas::mousePressEvent(QMouseEvent *e)
{
    sf::Vector2i pixelCoord{e->pos().x(), e->pos().y()};
    sf::Vector2f viewCoord{mapPixelToCoords(pixelCoord, mCameraView)};
    mMainController->mousePress(ophidian::geometry::Point(viewCoord.x, viewCoord.y));
}

void MySFMLCanvas::mouseMoveEvent(QMouseEvent *e)
{
    sf::Vector2i pixelCoord{e->pos().x(), e->pos().y()};
    sf::Vector2f viewCoord{mapPixelToCoords(pixelCoord, mCameraView)};
    mMainController->mouseMove(ophidian::geometry::Point(viewCoord.x, viewCoord.y));
}

void MySFMLCanvas::mouseReleaseEvent(QMouseEvent *e)
{
    mMainController->mouseRelease();
}

void MySFMLCanvas::centerViewOn(const ophidian::geometry::Point &p1)
{
    mCameraView.setCenter(sf::Vector2f(p1.x(), p1.y()));
}

void MySFMLCanvas::viewSize(const ophidian::geometry::Point &size)
{
    mCameraView.setSize(sf::Vector2f(size.x(), size.y()));
}

void MySFMLCanvas::reserveMinimumOfQuads(std::size_t minimumOfQuads)
{
    mCanvas.reserveMinimumOfQuads(minimumOfQuads);
}
