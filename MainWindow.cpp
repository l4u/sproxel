#include <QtGui>

#include <iostream>

#include "GLCamera.h"
#include "MainWindow.h"
#include "GLModelWidget.h"

#include <QFileDialog>
#include <QColorDialog>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // Windows
    m_glModelWidget = new GLModelWidget;
    setCentralWidget(m_glModelWidget);

    // The docking palette
    m_paletteDocker = new QDockWidget(tr("Palette"), this);
    m_paletteDocker->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_paletteWidget = new PaletteWidget(this);
    m_paletteDocker->setWidget(m_paletteWidget);
    addDockWidget(Qt::RightDockWidgetArea, m_paletteDocker);

    // Connect some window signals together
    QObject::connect(m_paletteWidget, SIGNAL(activeColorChanged(Imath::Color4f)),
                     m_glModelWidget, SLOT(setActiveColor(Imath::Color4f)));
    QObject::connect(m_glModelWidget, SIGNAL(colorSampled(Imath::Color4f)),
                     m_paletteWidget, SLOT(setActiveColor(Imath::Color4f)));


    // Toolbar
    m_toolbar = new QToolBar("Tools", this);
    m_toolbar->setOrientation(Qt::Vertical);
    addToolBar(Qt::LeftToolBarArea, m_toolbar);


    // Actions & Menus
    menuBar()->show();
    m_menuFile = menuBar()->addMenu("Fi&le");

    m_actFileNew = new QAction("&New", this);
    m_actFileNew->setShortcut(Qt::CTRL + Qt::Key_N);
    m_menuFile->addAction(m_actFileNew);
    connect(m_actFileNew, SIGNAL(triggered()),
            this, SLOT(newGrid()));

    m_menuFile->addSeparator();

    m_actFileOpen = new QAction("&Open", this);
    m_actFileOpen->setShortcut(Qt::CTRL + Qt::Key_O);
    m_menuFile->addAction(m_actFileOpen);
    connect(m_actFileOpen, SIGNAL(triggered()), 
            this, SLOT(openFile()));

    m_actFileSave = new QAction("&Save", this);
    m_actFileSave->setShortcut(Qt::CTRL + Qt::Key_S);
    m_menuFile->addAction(m_actFileSave);
    connect(m_actFileSave, SIGNAL(triggered()), 
            this, SLOT(saveFile()));

    m_menuFile->addSeparator();

    m_actQuit = new QAction("&Quit", this);
    m_actQuit->setShortcut(Qt::CTRL + Qt::Key_Q);
    m_menuFile->addAction(m_actQuit);
    connect(m_actQuit, SIGNAL(triggered()), 
            qApp, SLOT(quit()));


    // ------ edit menu
    m_menuEdit = menuBar()->addMenu("&Edit");

    m_actUndo = new QAction("Undo", this);
    m_actUndo->setShortcut(Qt::CTRL + Qt::Key_Z);
    m_menuEdit->addAction(m_actUndo);
    connect(m_actUndo, SIGNAL(triggered()),
            m_glModelWidget, SLOT(undo()));

    m_actRedo = new QAction("Redo", this);
    m_actRedo->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Z);
    m_menuEdit->addAction(m_actRedo);
    connect(m_actRedo, SIGNAL(triggered()),
            m_glModelWidget, SLOT(redo()));

    m_menuEdit->addSeparator();

    m_actShiftUp = new QAction("Shift up", this);
    m_actShiftUp->setShortcut(Qt::CTRL + Qt::Key_BracketRight);
    m_menuEdit->addAction(m_actShiftUp);
    connect(m_actShiftUp, SIGNAL(triggered()),
            this, SLOT(shiftUp()));

    m_actShiftDown = new QAction("Shift down", this);
    m_actShiftDown->setShortcut(Qt::CTRL + Qt::Key_BracketLeft);
    m_menuEdit->addAction(m_actShiftDown);
    connect(m_actShiftDown, SIGNAL(triggered()),
            this, SLOT(shiftDown()));

    m_actShiftWrap = new QAction("Wrap shift ops", this);
    m_actShiftWrap->setCheckable(true);
    m_actShiftWrap->setChecked(m_glModelWidget->shiftWrap());
    m_menuEdit->addAction(m_actShiftWrap);
    connect(m_actShiftWrap, SIGNAL(toggled(bool)),
            m_glModelWidget, SLOT(setShiftWrap(bool)));

    m_menuEdit->addSeparator();

    m_actUpRes = new QAction("Double grid resolution", this);
    m_actUpRes->setShortcut(Qt::CTRL + Qt::Key_Plus);
    m_menuEdit->addAction(m_actUpRes);
    connect(m_actUpRes, SIGNAL(triggered()),
            this, SLOT(upRes()));

    m_actDownRes = new QAction("Half grid resolution", this);
    m_actDownRes->setShortcut(Qt::CTRL + Qt::Key_Minus);
    m_menuEdit->addAction(m_actDownRes);
    connect(m_actDownRes, SIGNAL(triggered()),
            this, SLOT(downRes()));


    // ------ view menu
    m_menuView = menuBar()->addMenu("&View");

    m_actViewGrid = new QAction("View Grid", this);
    m_actViewGrid->setShortcut(Qt::CTRL + Qt::Key_G);
    m_actViewGrid->setCheckable(true);
    m_actViewGrid->setChecked(m_glModelWidget->drawGrid());
    m_menuView->addAction(m_actViewGrid);
    connect(m_actViewGrid, SIGNAL(toggled(bool)),
            m_glModelWidget, SLOT(setDrawGrid(bool)));

    m_actViewVoxgrid = new QAction("Voxel Grid", this);
    m_actViewVoxgrid->setShortcut(Qt::Key_G);
    m_actViewVoxgrid->setCheckable(true);
    m_actViewVoxgrid->setChecked(m_glModelWidget->drawVoxelGrid());
    m_menuView->addAction(m_actViewVoxgrid);
    connect(m_actViewVoxgrid, SIGNAL(toggled(bool)),
            m_glModelWidget, SLOT(setDrawVoxelGrid(bool)));

    m_actViewBBox = new QAction("Bounding Box", this);
    m_actViewBBox->setShortcut(Qt::CTRL + Qt::Key_B);
    m_actViewBBox->setCheckable(true);
    m_actViewBBox->setChecked(m_glModelWidget->drawBoundingBox());
    m_menuView->addAction(m_actViewBBox);
    connect(m_actViewBBox, SIGNAL(toggled(bool)),
            m_glModelWidget, SLOT(setDrawBoundingBox(bool)));


    // ------ window menu
    m_menuWindow = menuBar()->addMenu("&Window");
    m_menuWindow->addAction(m_toolbar->toggleViewAction());
    m_menuWindow->addAction(m_paletteDocker->toggleViewAction());


    // ------ toolbar hookups
    m_toolbarActionGroup = new QActionGroup(this);

    m_actToolSplat = new QAction("Splat", m_toolbarActionGroup);
    m_actToolSplat->setIcon(QIcon(QPixmap("./icons/splat.png")));
    m_actToolSplat->setCheckable(true);
    connect(m_actToolSplat, SIGNAL(toggled(bool)), this, SLOT(setToolSplat(bool)));
    
    m_actToolReplace = new QAction("Replace", m_toolbarActionGroup);
    m_actToolReplace->setIcon(QIcon(QPixmap("./icons/pencil.png")));
    m_actToolReplace->setCheckable(true);
    connect(m_actToolReplace, SIGNAL(toggled(bool)), this, SLOT(setToolReplace(bool)));
    
    m_actToolFlood = new QAction("Flood", m_toolbarActionGroup);
    m_actToolFlood->setIcon(QIcon(QPixmap("./icons/paintBucket.png")));
    m_actToolFlood->setCheckable(true);
    connect(m_actToolFlood, SIGNAL(toggled(bool)), this, SLOT(setToolFlood(bool)));
    
    m_actToolDropper = new QAction("Dropper", m_toolbarActionGroup);
    m_actToolDropper->setIcon(QIcon(QPixmap("./icons/eyeDropper.png")));
    m_actToolDropper->setCheckable(true);
    connect(m_actToolDropper, SIGNAL(toggled(bool)), this, SLOT(setToolDropper(bool)));
    
    m_actToolEraser = new QAction("Eraser", m_toolbarActionGroup);
    m_actToolEraser->setIcon(QIcon(QPixmap("./icons/eraser.png")));
    m_actToolEraser->setCheckable(true);
    connect(m_actToolEraser, SIGNAL(toggled(bool)), this, SLOT(setToolEraser(bool)));
    
    m_actToolSlab = new QAction("Slab", m_toolbarActionGroup);
    m_actToolSlab->setIcon(QIcon(QPixmap("./icons/slab.png")));
    m_actToolSlab->setCheckable(true);
    connect(m_actToolSlab, SIGNAL(toggled(bool)), this, SLOT(setToolSlab(bool)));

    //m_actToolRay = new QAction("Ray", this);
    
    m_actToolSplat->setChecked(true);
    m_toolbar->addActions(m_toolbarActionGroup->actions());


    // Remaining verbosity
    setWindowTitle(tr("Sproxel " SPROXEL_VERSION));
    statusBar()->showMessage(tr("Ready"));
}


void MainWindow::keyPressEvent(QKeyEvent* event)
{
    const bool altDown = event->modifiers() & Qt::AltModifier;
    const bool ctrlDown = event->modifiers() & Qt::ControlModifier;
    //const bool shiftDown = event->modifiers() & Qt::ShiftModifier;

    if (altDown && event->key() == Qt::Key_X)
    {
        m_glModelWidget->setCurrentAxis(GLModelWidget::X_AXIS);
    }
    else if (altDown && event->key() == Qt::Key_Y)
    {
        m_glModelWidget->setCurrentAxis(GLModelWidget::Y_AXIS);
    }
    else if (altDown && event->key() == Qt::Key_Z)
    {
        m_glModelWidget->setCurrentAxis(GLModelWidget::Z_AXIS);
    }
    else if (ctrlDown && event->key() == Qt::Key_C)
    {
        QColor color = QColorDialog::getColor(Qt::white, this);
        m_paletteWidget->setActiveColor(Imath::Color4f((float)color.red()/255.0f,
                                                       (float)color.green()/255.0f, 
                                                       (float)color.blue()/255.0f, 
                                                       (float)color.alpha()/255.0f));
    }
    else if (ctrlDown && event->key() == Qt::Key_F)
    {
        // Frame the full extents no matter what
        m_glModelWidget->frame(true);
    }
    else if (event->key() == Qt::Key_F)
    {
        // Frame the data if it exists
        m_glModelWidget->frame(false);
    }
    else if (event->key() == Qt::Key_X)
    {
        m_paletteWidget->swapColors();
    }
    //else if (event->key() == Qt::Key_D)
    //{
    //    m_paletteWidget->setDefaultColors();
    //}

    else if (event->key() == Qt::Key_Q) m_actToolSplat->setChecked(true);
    else if (event->key() == Qt::Key_W) m_actToolReplace->setChecked(true);
    else if (event->key() == Qt::Key_E) m_actToolFlood->setChecked(true);
    else if (event->key() == Qt::Key_R) m_actToolDropper->setChecked(true);
    else if (event->key() == Qt::Key_T) m_actToolEraser->setChecked(true);
    else if (event->key() == Qt::Key_Y) m_actToolSlab->setChecked(true);

    else if (event->key() >= Qt::Key_Left && event->key() <= Qt::Key_PageDown)
    {
        m_glModelWidget->handleArrows(event);
    }
    else if (event->key() == Qt::Key_Space)
    {
        // It's okay to call setVoxelColor once on the model widget, but any more requires an internal wrapper
        m_glModelWidget->setVoxelColor(m_glModelWidget->activeVoxel(), 
                                       m_glModelWidget->activeColor());
        m_glModelWidget->updateGL();
    }
    else if (event->key() == Qt::Key_Delete)
    {
        // It's okay to call setVoxelColor once on the model widget, but any more requires an internal wrapper
        m_glModelWidget->setVoxelColor(m_glModelWidget->activeVoxel(), 
                                       Imath::Color4f(0.0f, 0.0f, 0.0f, 0.0f));
        m_glModelWidget->updateGL();
    }
}


void MainWindow::newGrid()
{
    NewGridDialog dlg(this);

    dlg.setModal(true);
    if (dlg.exec())
    {
        m_glModelWidget->resizeVoxelGrid(dlg.getVoxelSize());
    }
}


void MainWindow::saveFile()
{
    QString filename = QFileDialog::getSaveFileName(this,
        tr("Save voxels (CSV format) file as..."),
        QString(""),
        tr("CSV Files (*.csv)"));
    if (!filename.isEmpty())
    {
        m_glModelWidget->saveGridCSV(filename.toStdString());
    }
}


void MainWindow::openFile()
{
    QString filename = QFileDialog::getOpenFileName(this,
        tr("Select CSV file to Open..."),
        QString(),
        tr("CSV Files (*.csv)"));
    if (!filename.isEmpty())
    {
        m_glModelWidget->loadGridCSV(filename.toStdString());
    }
}


// Trampoline functions because QSignalMapper can't do complex args 
// Search for QBoundMethod for a custom approach, but I'm too lazy to include it for now.
void MainWindow::shiftUp()
{
    m_glModelWidget->shiftVoxels(m_glModelWidget->currentAxis(), true, m_glModelWidget->shiftWrap());
}
void MainWindow::shiftDown()
{
    m_glModelWidget->shiftVoxels(m_glModelWidget->currentAxis(), false, m_glModelWidget->shiftWrap());
}

void MainWindow::upRes()   { m_glModelWidget->reresVoxelGrid(2.0f); }
void MainWindow::downRes() { m_glModelWidget->reresVoxelGrid(0.5f); }

void MainWindow::setToolSplat(bool stat)   { if (stat) m_glModelWidget->setActiveTool(GLModelWidget::TOOL_SPLAT); }
void MainWindow::setToolFlood(bool stat)   { if (stat) m_glModelWidget->setActiveTool(GLModelWidget::TOOL_FLOOD); }
void MainWindow::setToolRay(bool stat)     { if (stat) m_glModelWidget->setActiveTool(GLModelWidget::TOOL_RAY); }
void MainWindow::setToolDropper(bool stat) { if (stat) m_glModelWidget->setActiveTool(GLModelWidget::TOOL_DROPPER); }
void MainWindow::setToolEraser(bool stat)  { if (stat) m_glModelWidget->setActiveTool(GLModelWidget::TOOL_ERASER); }
void MainWindow::setToolReplace(bool stat) { if (stat) m_glModelWidget->setActiveTool(GLModelWidget::TOOL_REPLACE); }
void MainWindow::setToolSlab(bool stat)    { if (stat) m_glModelWidget->setActiveTool(GLModelWidget::TOOL_SLAB); }


// // Steal keypresses from your children!
// bool MainWindow::eventFilter(QObject* qo, QEvent* ev)
// {
//     if (ev->type() != QEvent::KeyPress) return false;
//     keyPressEvent(dynamic_cast<QKeyEvent*>(ev));
//     return true;
// }

