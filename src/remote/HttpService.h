/*
FatRat download manager
http://fatrat.dolezel.info

Copyright (C) 2006-2010 Lubos Dolezel <lubos a dolezel.info>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

In addition, as a special exemption, Luboš Doležel gives permission
to link the code of FatRat with the OpenSSL project's
"OpenSSL" library (or with modified versions of it that use the; same
license as the "OpenSSL" library), and distribute the linked
executables. You must obey the GNU General Public License in all
respects for all of the code used other than "OpenSSL".
*/

#ifndef HTTPSERVICE_H
#define HTTPSERVICE_H
#include "config.h"
#include <QThread>
#include <QMap>
#include <QByteArray>
#include <QVariantMap>
#include <QFile>
#include <ctime>
#include <pion/net/HTTPResponseWriter.hpp>
#include "remote/TransferHttpService.h"

#ifndef WITH_WEBINTERFACE
#	error This file is not supposed to be included!
#endif

#include <pion/net/WebServer.hpp>

class Queue;
class Transfer;

class HttpService : public QObject
{
public:
	HttpService();
	~HttpService();
	static HttpService* instance() { return m_instance; }
	void applySettings();
	
	void setup();

	static void findQueue(QString queueUUID, Queue** q);
	static int findTransfer(QString transferUUID, Queue** q, Transfer** t, bool lockForWrite = false);
private:
	static HttpService* m_instance;
	pion::net::WebServer* m_server;

	class GraphService : public pion::net::WebService
	{
		void operator()(pion::net::HTTPRequestPtr &request, pion::net::TCPConnectionPtr &tcp_conn);
	};
	class QgraphService : public pion::net::WebService
	{
		void operator()(pion::net::HTTPRequestPtr &request, pion::net::TCPConnectionPtr &tcp_conn);
	};
	class LogService : public pion::net::WebService
	{
		void operator()(pion::net::HTTPRequestPtr &request, pion::net::TCPConnectionPtr &tcp_conn);
	};
	class TransferTreeBrowserService : public pion::net::WebService
	{
		void operator()(pion::net::HTTPRequestPtr &request, pion::net::TCPConnectionPtr &tcp_conn);
	};
	class TransferDownloadService : public pion::net::WebService
	{
		void operator()(pion::net::HTTPRequestPtr &request, pion::net::TCPConnectionPtr &tcp_conn);
	};
	class SubclassService : public pion::net::WebService, public TransferHttpService::WriteBack
	{
	public:
		void write(const char* data, size_t bytes);
		void writeFail(QString error);
		void setContentType(const char* type);
		void operator()(pion::net::HTTPRequestPtr &request, pion::net::TCPConnectionPtr &tcp_conn);
	private:
		pion::net::HTTPResponseWriterPtr m_writer;
	};
};


#endif
