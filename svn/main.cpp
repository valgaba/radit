#include "mainwindow.h"

#include <QApplication>
#include <QIcon>
#include <QSplashScreen>
#include <QPixmap>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QSettings>
#include "core/SpanishTranslator.h"


/*int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile file(QDir(QCoreApplication::applicationDirPath())
                   .filePath(QStringLiteral("radit.qss")));
    file.open(QFile::ReadOnly);
   // QString styleSheet = QLatin1String(file.readAll());
    QString styleSheet = QString::fromUtf8(file.readAll());  // Usa fromUtf8() para mejor compatibilidad


    qApp->setStyleSheet(styleSheet);
    //  a.setStyleSheet(styleSheet);



    QPixmap pixmap(":/icons/splashscreen.png");
    QSplashScreen *splash = new QSplashScreen(pixmap);
    splash->show();

    splash->setCursor(QCursor(Qt::WaitCursor));



    QCoreApplication::processEvents();

    MainWindow w;


    splash->finish(&w);
    splash->setCursor(QCursor(Qt::ArrowCursor));

    w.showMaximized();


    delete splash;

    return a.exec();
}*/

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    SpanishTranslator spanishTranslator;
    const QString settingsPath=QDir(QCoreApplication::applicationDirPath())
                                   .filePath(QStringLiteral("interface.ini"));
    QSettings settings(settingsPath,QSettings::IniFormat);
    if (settings.value(QStringLiteral("Interface/language"),QStringLiteral("en")).toString()
        ==QLatin1String("es"))
        a.installTranslator(&spanishTranslator);
    a.setWindowIcon(QIcon(":/icons/radit.ico"));

    //  Cargar y aplicar la hoja de estilos (QSS)
    QFile file("radit.qss");
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QString::fromUtf8(file.readAll());  // Usa UTF-8 para mejor compatibilidad
        a.setStyleSheet(styleSheet);  // Aplicar el QSS
        file.close();
    } else {
        qWarning() << " No se pudo abrir el archivo radit.qss";
    }

    //  Cargar la imagen del splash screen
    QPixmap pixmap(":/icons/splashscreen.png");
    if (pixmap.isNull()) {
        qWarning() << " No se pudo cargar la imagen del splash screen.";
    }

    QSplashScreen splash(pixmap);
    splash.show();
    splash.setCursor(Qt::WaitCursor);

    QCoreApplication::processEvents();  // Procesa eventos mientras el splash está visible

    //  Mostrar la ventana principal
    MainWindow w;
    splash.finish(&w);
    w.showMaximized();  // Mostrar maximizado

    return a.exec();
}





