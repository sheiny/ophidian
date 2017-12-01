#include "maincontroller.h"

MainController::MainController()
{
    srand((unsigned)time(0));
}

MainController::~MainController()
{
    delete mBuilder;
}

void MainController::setCanvas(Canvas * canvas)
{
    mCanvas = canvas;
}

void MainController::selectedCell(const ophidian::circuit::Cell & cell)
{
    std::string name = mDesign->netlist().name(cell);
    std::string type = mDesign->standardCells().name( mDesign->libraryMapping().cellStdCell(cell) );
    ophidian::geometry::Point origin = mDesign->placement().cellLocation(cell).toPoint();

    auto size = cellSize(cell);

    emit on_selected_cellChanged(QString::fromStdString(name), QString::fromStdString(type), size.x(), size.y(), origin.x(), origin.y());
}

void MainController::mousePress(const ophidian::geometry::Point & p)
{
    if (mIndex.hasQuad(p))
        selectedCell(mIndex.quadContaining(p).mCell);
    else
        emit on_selected_cellChanged("", "", 0, 0, p.x(), p.y());
}

void MainController::mouseMove(const ophidian::geometry::Point & p)
{
    emit on_selected_cellChanged("moving", "moving", -1, -1, p.x(), p.y());
}

void MainController::buildICCAD2017(std::string cells_lef, std::string tech_lef, std::string placed_def)
{
    if (mBuilder != nullptr)
    {
        delete mBuilder;
        mCanvas->clear();
    }

    /* Building */
    mBuilder = new ophidian::design::ICCAD2017ContestDesignBuilder(cells_lef, tech_lef, placed_def);
    init();
}

void MainController::buildICCAD2015(std::string lef, std::string def, std::string verilog)
{
    if (mBuilder != nullptr)
    {
        delete mBuilder;
        mCanvas->clear();
    }

    /* Building */
    mBuilder = new ophidian::design::ICCAD2015ContestDesignBuilder(lef, def, verilog);
    init();
}

void MainController::init()
{
    mDesign = & mBuilder->build();

    /* Creating property Cells to Quads */
    mCellToQuads = mDesign->netlist().makeProperty<std::vector<Quad>>(ophidian::circuit::Cell());

    /* Getting information of netlist */
    size_t cells = mDesign->netlist().size(ophidian::circuit::Cell());
    size_t pins = mDesign->netlist().size(ophidian::circuit::Pin());
    size_t nets = mDesign->netlist().size(ophidian::circuit::Net());

    ophidian::geometry::Point chipUpper = mDesign->floorplan().chipUpperRightCorner().toPoint();

    std::stringstream concat;
    concat << chipUpper.x() << "μ X " << chipUpper.y() << "μ";
    std::string dieArea = concat.str();

    /* Getting name of file
    size_t pos = 0;
    std::string name;
    while ((pos = lef.find("/")) != std::string::npos) {
        name = lef.substr(0, pos);
        lef.erase(0, pos + 1);
    }
    name = lef.substr(0, lef.find("."));
    */

    /* Saying to MainWindow display the information */
    emit on_circuit_labelsChanged("Circuit", QString::fromStdString(dieArea), cells, pins, nets);

    createQuads();
}

void MainController::createQuads()
{
    // Chip Area
    mCanvas->createBoundaries(mDesign->floorplan().chipUpperRightCorner().toPoint());

    // Cells
    for (auto cellIt = mDesign->netlist().begin(ophidian::circuit::Cell()); cellIt != mDesign->netlist().end(ophidian::circuit::Cell()); cellIt++)
    {

        std::vector<Quad> quads;

        ophidian::geometry::MultiBox cellGeometry = mDesign->placementMapping().geometry(*cellIt);

        for (auto cellBoxIt = cellGeometry.begin(); cellBoxIt != cellGeometry.end(); cellBoxIt++)
        {
            Quad quad(*cellIt);
            std::vector<ophidian::geometry::Point> points;

            points.push_back( ophidian::geometry::Point( (*cellBoxIt).min_corner().x(), (*cellBoxIt).min_corner().y()) );
            points.push_back( ophidian::geometry::Point( (*cellBoxIt).max_corner().x(), (*cellBoxIt).min_corner().y()) );
            points.push_back( ophidian::geometry::Point( (*cellBoxIt).max_corner().x(), (*cellBoxIt).max_corner().y()) );
            points.push_back( ophidian::geometry::Point( (*cellBoxIt).min_corner().x(), (*cellBoxIt).max_corner().y()) );

            mCanvas->alloc(quad, points);
            mIndex.quadCreate(quad, *cellBoxIt);

            quads.push_back(quad);
        }

         mCellToQuads[*cellIt] = quads;

        if (isFixed(*cellIt))
        {
            mCanvas->paint(quads, sf::Color::Blue);
        }
        else
        {
            unsigned int random = rand();
            mCanvas->paint(quads, sf::Color((random % 10 + 195), (random % 75), (random % 120 + 135)));
        }
    }
}

bool MainController::isFixed(const ophidian::circuit::Cell & cell)
{
    return mDesign->placement().isFixed(cell);
}

bool MainController::hasCell(std::string name)
{
    if (mBuilder == nullptr)
        return false;

    auto cell = mDesign->netlist().find(ophidian::circuit::Cell(), name);
    try {
        mDesign->placement().isFixed(cell);
        return true;
    } catch (const std::out_of_range & e) {
        return false;
    }
}

bool MainController::hasQuad(const ophidian::geometry::Point & p)
{
    return mIndex.hasQuad(p);
}

Quad MainController::quadAt(const ophidian::geometry::Point & p)
{
    return mIndex.quadContaining(p);
}

std::vector<Quad> MainController::quadsCell(const ophidian::circuit::Cell & cell)
{
    return mCellToQuads[cell];
}

std::vector<Quad> MainController::quadsCell(std::string name)
{
    if (mBuilder == nullptr)
        return std::vector<Quad>();

    auto cell = mDesign->netlist().find(ophidian::circuit::Cell(), name);
    try {
        return mCellToQuads[cell];
    } catch (const std::out_of_range & e) {
        return std::vector<Quad>();
    }
}

WireQuad MainController::wireQuadOfCell(const ophidian::circuit::Cell & cell)
{
    return mCanvas->createWireQuad(cell, mDesign->placement().cellLocation(cell).toPoint(), cellSize(cell));
}

void MainController::clear(WireQuad & wire)
{
    mCanvas->clear(wire);
}

void MainController::transform(Quad quad, const sf::Transform & trans)
{
    mCanvas->transform(mCellToQuads[quad.mCell], trans);
}

void MainController::transform(WireQuad wire, const sf::Transform & trans)
{
    std::vector<Line> lines;
    for (const auto & l : wire.mLines)
        lines.push_back(l);

    mCanvas->transform(lines, trans);
}

void MainController::update(Quad quad)
{
    Quad first = mCellToQuads[quad.mCell].front();
    auto newOrigin = mCanvas->points(first).front();

    mDesign->placement().placeCell(first.mCell, ophidian::util::LocationDbu(newOrigin.position.x, newOrigin.position.y));

    ophidian::geometry::MultiBox cellGeometry = mDesign->placementMapping().geometry(first.mCell);

    auto quadOfBox = mCellToQuads[first.mCell].begin();
    for (auto cellBoxIt = cellGeometry.begin(); cellBoxIt != cellGeometry.end(); cellBoxIt++, quadOfBox++)
    {
        mIndex.quadRemove(*quadOfBox);
        mIndex.quadCreate(*quadOfBox, *cellBoxIt);
    }
}

void MainController::remove(Quad quad, WireQuad wire)
{
    auto quads = mCellToQuads[quad.mCell];
    mCanvas->desalloc(quads);

    std::vector<Line> lines;
    for (const auto & l : wire.mLines)
        lines.push_back(l);
    mCanvas->desalloc(lines);

    auto quadsOfBox = mCellToQuads[quad.mCell];
    for (auto quadIt = quadsOfBox.begin(); quadIt != quadsOfBox.end(); quadIt++)
    {
        mIndex.quadRemove(*quadIt);
    }

    mDesign->netlist().erase(quad.mCell);
    /* Just hedge the points to the origin, not to need to
     * find a cell to put in place (quantities of different quads).
     */

    size_t cells = mDesign->netlist().size(ophidian::circuit::Cell());
    size_t pins = mDesign->netlist().size(ophidian::circuit::Pin());
    size_t nets = mDesign->netlist().size(ophidian::circuit::Net());
    emit on_circuit_labelsChanged("removing", "removing", cells, pins, nets);
}

ophidian::geometry::Point MainController::chipBoundaries()
{
    return mDesign->floorplan().chipUpperRightCorner().toPoint();
}

void MainController::slot1()
{
    /* - Implement your algorithm or functionality here.
     */

    if (mBuilder != nullptr)
        updateAfterSlots();
}

void MainController::slot2()
{
    /* - Implement your algorithm or functionality here.
     */

    if (mBuilder != nullptr)
        updateAfterSlots();
}

void MainController::slot3()
{
    /* - Implement your algorithm or functionality here.
     */

    if (mBuilder != nullptr)
        updateAfterSlots();
}

void MainController::updateAfterSlots()
{
    mCanvas->clear();
    mIndex.clear();
    createQuads();
}

ophidian::geometry::Point MainController::cellSize(const ophidian::circuit::Cell & cell)
{
    unsigned int width = 0, height = 0;
    ophidian::geometry::MultiBox cellGeometry = mDesign->placementMapping().geometry(cell);
    for (auto cellBoxIt = cellGeometry.begin(); cellBoxIt != cellGeometry.end(); cellBoxIt++)
    {
        if (width < ((*cellBoxIt).max_corner().x() - (*cellBoxIt).min_corner().x()))
            width = (*cellBoxIt).max_corner().x() - (*cellBoxIt).min_corner().x();

        height += (*cellBoxIt).max_corner().y() - (*cellBoxIt).min_corner().y();
    }

    return ophidian::geometry::Point(width, height);
}