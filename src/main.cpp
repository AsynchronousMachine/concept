#include "modules/global_modules.hpp"
#include "modules/maker_reflection.hpp"

#include "udp/udpCommand.h"

#include <fstream>

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

void sendConfigFile()
{
	boost::asio::io_service udpIOService;
	Asm::UdpServer* udpServer = new Asm::UdpServer(udpIOService);

	/*std::string content = readFile("output-test.json");
	std::cout << content;
	udpServer->sendTo("127.0.0.1", 9500, content);*/

	std::string content = readFile("output-link.json");
	std::cout << content;
	udpServer->sendTo("127.0.0.1", 9501, content);

	//udpServer->sendTo("127.0.0.1", 9500, "{	\"hello\": \"world\",		\"t\": true ,		\"f\": false,		\"n\": null,		\"i\": 123,		\"pi\": 3.1416,		\"a\": [1, 2, 3, 4]}");

}

std::unique_ptr<Asm::DataObjectReactor> rptr (new Asm::DataObjectReactor());
//void testlauf()
//{
//	//init receiver
//	Asm::UdpCommand command;
//
//	boost::asio::io_service udpIOService;
//	std::unique_ptr<Asm::UdpServer> udpServer (new Asm::UdpServer(udpIOService));
//
//	//setlink
//	udpServer->sendTo("127.0.0.1", 9501, "{\"Module2.link1\" :[\"LINK1\",\"Module1.m1.do1\",\"Module2.m2.do1\"]}");
//	// wait till link is set
//	boost::this_thread::sleep_for(boost::chrono::seconds(2));
//
//	//read m2.do1
//	std::cout << "m1.do1=" << m1.do1.get([](auto i) {return i; }) << "  m2.do1=" << m2.do1.get([](auto i) {return i; }) << std::endl;
//
//	//now update m1.do1
//	m1.do1.setAndTrigger([](int &i) { i = 5; }, *rptr);
//	boost::this_thread::sleep_for(boost::chrono::seconds(2));
//
//	//read m2.do1 again
//	std::cout << "m1.do1=" << m1.do1.get([](auto i) {return i; }) << "  m2.do1=" << m2.do1.get([](auto i) {return i; }) << std::endl;
//
//	//clearlink
//	udpServer->sendTo("127.0.0.1", 9501, "{\"Module2.link1\" :[\"LINK1\",\"Module1.m1.do1\"]}");
//	boost::this_thread::sleep_for(boost::chrono::seconds(2));
//
//	//now update m1.do1
//	m1.do1.setAndTrigger([](int &i) { i = 8; }, *rptr);
//
//
//	/*
//	TODO trigger ist nicht mit dem Datentyp boost::variant<EmptyDataobject&, ...> ausführbar,
//	deshalb DataObject um eine Funktion setAndTrigger erweitern, die selbst trigger auslöst
//
//    rptr->trigger(dataobject_map.at("Module1.m1.do1"));
//	boost::apply_visitor([&](auto &d) {rptr->trigger(d);}, dataobject_map.at("Module1.m1.do1"));
//	*/
//
//	boost::this_thread::sleep_for(boost::chrono::seconds(2));
//
//	//read m2.do1 again
//	std::cout << "m1.do1=" << m1.do1.get([](auto i) {return i; }) << "  m2.do1=" << m2.do1.get([](auto i) {return i; }) << std::endl;
//
//}


int main() {
	my.initdo();
	my.initdo2();
	sys.deserialize();

	std::cout << my.myDO.get([](auto& i) {return i; }).inputCounter << my.myDO.get([](auto& i) {return i; }).outputCounter << my.myDO.get([](auto& i) {return i; }).message << std::endl;
	std::cout << my.myDO.get([](auto& i) {return &i; })<< std::endl;
	std::cout << my.myDO.get([](auto& i) {return &i; }) << std::endl;
	//testlauf();
	return 0;

	//start command service as thread for non-blocking at listener
	Asm::UdpCommand command;

	//init DO values
	sys.deserialize();

	//set and read DO values
	sendConfigFile();

	//have to wait before exiting the program
	boost::this_thread::sleep_for(boost::chrono::seconds(2));

	exit(0);



	//sys.walkthrough();



	//sys.serialize();
	//sys.deserialize();



	//WARNING: get is NOT usable with variant
	//boost::apply_visitor([&](auto& d) ->auto{return d.get([](auto i)->auto {return i; }); }, dataobject_map.at("Module1.m1.do1"));

	//boost::get<Asm::DataObject<int>&>(dataobject_map.at("Module1.m1.do1")).get([](int i)->int {std::cout << i; return i; });

	//boost::apply_visitor([&](auto& d) { d.serialize(json_value, doc.GetAllocator()); }, map_iter.second);

	//boost::apply_visitor(t_visitor_get(), dataobject_map.at("Module1.m1.do1"));
	//(.get<>.get([](int i)->int {std::cout << i; return i; });

	//int ii = boost::apply_visitor([](auto& d)->auto {return d.get([](auto i)->int { return i; }); }, dataobject_map.at("Module1.m1.do1"));



}
