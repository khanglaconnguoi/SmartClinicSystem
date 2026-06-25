#include "repository/DatabaseManager.h"
#include "ui/MainWindow.h"
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QSqlDatabase>

// Log handler ghi ra file để debug (vì WIN32 app không có console)
static QFile logFile;

void messageHandler(QtMsgType type, const QMessageLogContext &ctx,
                    const QString &msg) {
  Q_UNUSED(ctx);
  if (!logFile.isOpen())
    return;

  QString prefix;
  switch (type) {
  case QtDebugMsg:
    prefix = "DEBUG";
    break;
  case QtWarningMsg:
    prefix = "WARN ";
    break;
  case QtCriticalMsg:
    prefix = "ERROR";
    break;
  case QtFatalMsg:
    prefix = "FATAL";
    break;
  default:
    prefix = "INFO ";
    break;
  }

  QTextStream out(&logFile);
  out << prefix << ": " << msg << "\n";
  out.flush();
}

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  // Ghi log ra file cùng thư mục exe
  logFile.setFileName(app.applicationDirPath() + "/debug.log");
  if (!logFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    // Log file không mở được — không ảnh hưởng chức năng chính,
    // messageHandler đã kiểm tra isOpen() nên sẽ tự bỏ qua.
    qWarning() << "Cannot open log file:" << logFile.fileName();
  }
  qInstallMessageHandler(messageHandler);

  qDebug() << "=== SmartClinicSystem started ===";
  qDebug() << "Available SQL drivers:" << QSqlDatabase::drivers();

  // Khởi tạo CSDL
  DatabaseManager::getInstance();

  qDebug() << "Database connection OK";

  MainWindow window;
  window.show();

  int result = app.exec();

  qDebug() << "=== SmartClinicSystem exited ===";
  logFile.close();
  return result;
}