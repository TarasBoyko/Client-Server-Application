#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <mutex>

using std::vector;
using std::string;
using std::fstream;
using std::mutex;
using std::try_lock;


static const int maxNumberOfClient = 1000;

class Server
{
public:
	Server();
    int StartServer();
private:
	SOCKET my_socket_;
	sockaddr_in local_address_;

	int number_of_clients_; // number current clients, that connedced to server
	mutex number_of_clients_mutex_;
	vector<string> editing_files_; // names of files, that editing by some clients
	mutex editing_files_mutex_;

	int Initialization();
	int Binding();
	int Listening();
    int ServiceClient(void* clientSocket);
	int SendPieceOfFile(char* pBuffer, int& bufferLength, char* fileName, fstream& File, SOCKET& Socket);
    void UpdateFile(char* pBuffer, int& bufferLength, const char* fileName, std::fstream& File, SOCKET& mySocket);
    int WaitForClientsConnection();

    void AddClient();
    void SubtractClient();
    bool AddEditingFile(const string& newEditingFile);
    void SubtractEditingFile(const string& editingFile);

};

#endif // SERVER_H_INCLUDED
