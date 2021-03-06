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

#ifndef JTRANSFERPLUGIN_H
#define JTRANSFERPLUGIN_H

#include "config.h"
#ifndef WITH_JPLUGINS
#	error This file is not supposed to be included!
#endif

#include "JPlugin.h"
#include "Transfer.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMap>

class JavaDownload;

class JTransferPlugin : public JPlugin
{
	Q_OBJECT
public:
	JTransferPlugin(const JClass& cls, const char* sig = "()V", JArgs args = JArgs());
	JTransferPlugin(const char* clsName, const char* sig = "()V", JArgs args = JArgs());

	static void setMessage(JNIEnv *, jobject, jstring);
	static void setState(JNIEnv *, jobject, jobject);
	static void logMessage(JNIEnv *, jobject, jstring);
	static void setPersistentVariable(JNIEnv *, jobject, jstring, jobject);
	static jobject getPersistentVariable(JNIEnv *, jobject, jstring);

	virtual void setPersistentVariable(QString key, QVariant value) = 0;
	virtual QVariant getPersistentVariable(QString key) = 0;

	virtual void abort();

	static void registerNatives();
protected:
	class JStateEnum : public JObject
	{
	public:
		JStateEnum(jobject obj);
		Transfer::State value() const;
	};
};

#endif // JTRANSFERPLUGIN_H
