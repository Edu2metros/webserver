#ifndef CONTENTMAKER_H
#define CONTENTMAKER_H

#include <iostream>
#include <string>
#include <cstring>

using namespace std;

class ContentMaker {
	private:
		int _client;
		string _protocol, _connection, _status;
		void *_data;
		size_t _len;

	public:
		ContentMaker() : _client(0), _protocol(""), _connection(""), _status(""), _data(NULL), _len(0) {}

		ContentMaker(int client, string protocol, string connection, string status, void *data, size_t len)
			: _client(client), _protocol(protocol), _connection(connection), _status(status), _data(data), _len(len) {}

		ContentMaker(const ContentMaker &other) {
			_client = other._client;
			_protocol = other._protocol;
			_connection = other._connection;
			_status = other._status;
			_data = other._data;
			_len = other._len;
		}

		~ContentMaker() {}

		ContentMaker &operator=(const ContentMaker &other) {
			if (this != &other) {
				_client = other._client;
				_protocol = other._protocol;
				_connection = other._connection;
				_status = other._status;
				_data = other._data;
				_len = other._len;
			}
			return *this;
		}

		bool operator==(const ContentMaker &other) const {
			return _client == other._client &&
				   _protocol == other._protocol &&
				   _connection == other._connection &&
				   _status == other._status &&
				   _data == other._data &&
				   _len == other._len;
		}

		bool operator!=(const ContentMaker &other) const {
			return !(*this == other);
		}

		int getClient() const { return _client; }
		string getProtocol() const { return _protocol; }
		string getConnection() const { return _connection; }
		string getStatus() const { return _status; }
		void *getData() const { return _data; }
		size_t getLen() const { return _len; }

		void setClient(int client) { _client = client; }
		void setProtocol(const string &protocol) { _protocol = protocol; }
		void setConnection(const string &connection) { _connection = connection; }
		void setStatus(const string &status) { _status = status; }
		void setData(void *data) { _data = data; }
		void setLen(size_t len) { _len = len; }

		void printContent() const {
			cout << "Client: " << _client << endl;
			cout << "Protocol: " << _protocol << endl;
			cout << "Connection: " << _connection << endl;
			cout << "Status: " << _status << endl;
			cout << "Data: " << (_data != NULL ? "Data Present" : "No Data") << endl;
			cout << "Length: " << _len << endl;
		}
};

#endif
