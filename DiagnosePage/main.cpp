#include "DiagnosePage.h"
#include <QtWidgets/QApplication>
#include "Global.h"
#include "DealSettingFile.h"
#include <QQueue>
#include <QMutex>
#include <QMetaType>
#include <boost/thread/condition.hpp>  
#include <boost/thread/mutex.hpp>  
#include <boost/atomic.hpp>   

bool CAN_open = false;
QQueue<QVector<VCI_CAN_OBJ> > send_can_data_queue;
QMutex send_queue_mutex;
QMutex record_mutex;
QQueue<Record> CAN_Record;
FunctionBlock current_page_block;

boost::mutex data_thread_mutex;
boost::condition_variable_any heart_thread_condition;
boost::atomic_int current_thread_order(0);

bool realtimedata_is_running = false;
DealSettingFile deal_settingfile;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	qRegisterMetaType<VCI_CAN_OBJ>("VCI_CAN_OBJ");
	qRegisterMetaType<TreeData>("TreeData");
	qRegisterMetaType<ECUTableData>("ECUTableData");
	qRegisterMetaType<InputWriteECUData>("InputWriteECUData");
	qRegisterMetaType<FaultTableData>("FaultTableData");
	qRegisterMetaType<SnapShotInfo>("SnapShotInfo");
	qRegisterMetaType<Exist_Device>("Exist_Device");
	qRegisterMetaType<FaultSortTableData>("FaultSortTableData");

    DiagnosePage w;
    w.show();
    return a.exec();
}
