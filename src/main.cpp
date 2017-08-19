/*
** Global entry for all test cases
*/

/*
** Hint: for Windows systems "WIN32_LEAN_AND_MEAN" has to be defined in preprocessor else you will get boost/asio/detail/socket_types.hpp(24): fatal error C1189: #error:  WinSock.h has already been included
*/



#ifdef __linux__
#include <pthread.h>
#include <sys/syscall.h>
#endif

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

#include <tbb/tbb.h>

#include "communication/ReceiveHandler.hpp"
#include "communication/TcpServer.hpp"
//#include "communication/tcp/tcpSyncClient.hpp"

extern void runDOAccessExamples();
extern void runDOReactorExamples();
extern void runModuleUsageExamples();
extern void runDOTimerExamples();
extern void runDOSerializationExamples();
extern void runTBBUsageExamples();

struct Observer : tbb::task_scheduler_observer {
	static constexpr int RT_PRIO = 30; // Realtime priority
	int _rt_prio;

	Observer(bool b = true, int rt_prio = RT_PRIO) : _rt_prio(rt_prio) {
		observe(b);
	}

	void on_scheduler_entry(bool) {
#ifdef __linux__
		pthread_t pid = pthread_self();
		pid_t tid = syscall(SYS_gettid);

		// The thread name is a meaningful C language string, whose length is
		// restricted to 16 characters, including the terminating null byte ('\0')
		std::string s = "TBB-TID-" + std::to_string(tid);
		std::cout << s << std::endl;

		if (pthread_setname_np(pid, s.data()))
			std::cout << "Could not set tbb names" << std::endl;

		struct sched_param param = {};
		param.sched_priority = _rt_prio;

		if (pthread_setschedparam(pid, SCHED_FIFO, &param))
			std::cout << "Could not set realtime parameter" << std::endl;
#endif
	}

	void on_scheduler_exit(bool) {
#ifdef __linux__
		char tn[20];
		pthread_getname_np(pthread_self(), &tn[0], sizeof(tn));
		std::cout << "TBB-Exit: " << tn << std::endl;
#endif
	}
};


std::string readFile(std::string filename)
{
	std::string contents;
	std::ifstream in(filename, std::ios::in | std::ios::binary);
	if (in)
	{
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
	}
	return(contents);
}

bool _runClient = true;

//void simulateClient() {
//	boost::thread* simClientThread = new boost::thread([&]() {
//		Asm::TCP_SyncClient client("127.0.0.1", 9601);
//		while (_runClient) {
//			std::string msg = readFile("input-link.json");
//			client.send(msg);
//			boost::this_thread::sleep_for(boost::chrono::seconds(5));
//		}
//	});
//}
//
//void simulateClientWithResponse() {
//	boost::thread* simClientThread = new boost::thread([&]() {
//		Asm::TCP_SyncClient client("127.0.0.1", 9600);
//		while (_runClient) {
//			std::string msg = readFile("input-do.json");
//			std::string response = client.sendAndRead(msg);
//			std::cout << "CLIENT Response: " << response << "\n";
//
//			boost::this_thread::sleep_for(boost::chrono::seconds(5));
//		}
//	});
//}



void readCallback(boost::asio::ip::tcp::socket& socket, size_t len, std::array<char, Asm::TcpServer::MAX_BUFFER_SIZE>& _buffer)
{
    std::cout << "Got " << len << " bytes: " << _buffer[0] << _buffer[1] << _buffer[2] << _buffer[3] << _buffer[4] << _buffer[5] << std::endl;

    std::string message{"OK\n"};
    socket.write_some(boost::asio::buffer(message));

    std::cout << "Did write" << std::endl;
}

int main() {
	Observer observer;
	tbb::task_scheduler_init tbb_init;

	// Wait for all instantiation processes to finish
	std::this_thread::sleep_for(std::chrono::seconds(5));

#ifdef __linux__
	std::cout << "TID of main: " << syscall(SYS_gettid) << std::endl;
#endif

	std::cout << "TBB threads, max available: " << tbb::task_scheduler_init::default_num_threads() << std::endl;

	//runDOAccessExamples();
	//runDOReactorExamples();
	//runDOTimerExamples();
	//runModuleUsageExamples();
	//runDOSerializationExamples();
	//runTBBUsageExamples();

	{
        std::unique_ptr<Asm::TcpServer> pDOServer = std::make_unique<Asm::TcpServer>(9600, readCallback);
        boost::this_thread::sleep_for(boost::chrono::seconds(20));
    }

	//std::unique_ptr<Asm::TCP_SyncServer> pLinkServer = std::make_unique<Asm::TCP_SyncServer>(9601, Asm::receivedLinkHandler);

//	try {
//		simulateClient();
//		boost::this_thread::sleep_for(boost::chrono::seconds(2));
//		simulateClientWithResponse();
//		//boost::this_thread::sleep_for(boost::chrono::seconds(30));
//		//_runClient = false;
//	}
//	catch (std::exception& e) {
//		std::cerr << "Exception:: " << e.what() << std::endl;
//	}

	std::cout << "===================================================================" << std::endl;
	std::cout << "Enter \'q\' for quit tests!" << std::endl;
	char c;
	std::cin >> c;

	exit(0);
}
