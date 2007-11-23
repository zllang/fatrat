#include "RapidTools.h"
#include "MainWindow.h"
#include "fatrat.h"
#include <QUrl>
#include <QMessageBox>
#include <QtDebug>

RapidTools::RapidTools()
	: m_httpRShare(0), m_httpRSafe(0), m_httpRF(0)
{
	setupUi(this);
	
	connect(pushCheck, SIGNAL(clicked()), this, SLOT(checkRShareLinks()));
	connect(pushDownload, SIGNAL(clicked()), this, SLOT(downloadRShareLinks()));
	
	connect(pushDecode, SIGNAL(clicked()), this, SLOT(decodeRSafeLinks()));
	connect(pushDownload2, SIGNAL(clicked()), this, SLOT(downloadRSafeLinks()));
	
	connect(pushExtract, SIGNAL(clicked()), this, SLOT(extractRFLinks()));
	connect(pushDownloadFolder, SIGNAL(clicked()), this, SLOT(downloadRFLinks()));
}

void RapidTools::checkRShareLinks()
{
	QStringList links;
	QByteArray data = "urls=";
	
	m_strRShareWorking.clear();
	links = textLinks->toPlainText().split('\n', QString::SkipEmptyParts);
	
	if(!links.isEmpty())
	{
		QRegExp re("http://rapidshare.com/files/(\\d+)/");
		
		foreach(QString link, links)
		{
			if(re.indexIn(link) < 0)
			{
				QMessageBox::warning(this, "FatRat", tr("An invalid link has been encountered: %1").arg(link));
				return;
			}
			
			m_mapRShare[re.cap(1).toULong()] = link;
			
			data += QUrl::toPercentEncoding(link);
			data += "%0D%0A";
		}
		
		m_httpRShare = new QHttp("rapidshare.com", 80, this);
		m_bufRShare = new QBuffer(m_httpRShare);
		connect(m_httpRShare, SIGNAL(done(bool)), this, SLOT(doneRShare(bool)));
		m_httpRShare->post("/cgi-bin/checkfiles.cgi", data, m_bufRShare);
		
		pushCheck->setDisabled(true);
	}
}

void RapidTools::downloadRShareLinks()
{
	if(!m_strRShareWorking.isEmpty())
	{
		MainWindow* wnd = (MainWindow*) getMainWindow();
		wnd->addTransfer(m_strRShareWorking);
	}
}

void RapidTools::doneRShare(bool error)
{
	if(error)
	{
		QMessageBox::critical(this, "FatRat", tr("Server failed to process our query."));
	}
	else
	{
		QRegExp re("<font color=\"([^\"]+)\">File (\\d+) ");
		
		QString result;
		
		const QByteArray& data = m_bufRShare->data();
		int pos = data.indexOf(")</script>");
		
		while(true)
		{
			pos = re.indexIn(data, pos);
			
			if(pos < 0)
				break;
			
			QString url = m_mapRShare[re.cap(2).toULong()];
			
			if(re.cap(1) == "green")
			{
				result += QString("<font color=green>%1</font><br>").arg(url);
				m_strRShareWorking += url + '\n';
			}
			else
			{
				result += QString("<font color=red>%1</font><br>").arg(url);
			}
			pos++;
		}
		
		qDebug() << result;
		textLinks->setHtml(result);
	}
	
	m_httpRShare->deleteLater();
	
	m_bufRShare = 0;
	m_httpRShare = 0;
	m_mapRShare.clear();
	
	pushCheck->setEnabled(true);
}

void RapidTools::decodeRSafeLinks()
{
	QStringList list = textLinksSrc->toPlainText().split('\n', QString::SkipEmptyParts);
	
	foreach(QString str, list)
	{
		if(str.startsWith("http://www.rapidsafe.net/"))
			str[18] = 'v';
		else if(!str.startsWith("http://www.rapidsave.net/"))
		{
			QMessageBox::warning(this, "FatRat", tr("An invalid link has been encountered: %1").arg(str));
			return;
		}
	}
	
	textLinksDst->clear();
	m_listRSafeSrc.clear();
	
	m_httpRSafe = new QHttp("www.rapidsave.net", 80, this);
	
	pushDecode->setDisabled(true);
	
	foreach(QString str, list)
	{
		QUrl url(str);
		int r;
		
		QBuffer* buf = new QBuffer(m_httpRSafe);
		connect(m_httpRSafe, SIGNAL(requestFinished(int,bool)), this, SLOT(doneRSafe(int,bool)));
		connect(m_httpRSafe, SIGNAL(done(bool)), this, SLOT(doneRSafe(bool)));
		r = m_httpRSafe->get(url.path(), buf);
		
		m_listRSafeSrc[r] = str;
	}
}

void RapidTools::downloadRSafeLinks()
{
	QString text = textLinksDst->toPlainText();
	if(!text.isEmpty())
	{
		MainWindow* wnd = (MainWindow*) getMainWindow();
		wnd->addTransfer(text);
	}
}

void RapidTools::doneRSafe(int r, bool error)
{
	QBuffer* buffer = (QBuffer*) m_httpRSafe->currentDestinationDevice();
	
	if(!error)
	{
		const QByteArray& data = buffer->data();
		int pos;
		QString result = "http://rapidshare.com";
		
		pos = data.indexOf("<FORM ACTION=\"http://rapidshare.com");
		if(pos >= 0)
		{
			pos += 35;
			
			while(data[pos] == '&')
			{
				int c = data.mid(pos+3, 2).toInt(0, 16);
				pos += 6;
				result += char(c);
			}
			
			textLinksDst->append(result);
			m_listRSafeSrc.remove(r);
		}
		else
			error = true;
	}
	
	if(error)
	{
		QMessageBox::critical(this, "FatRat", tr("Server failed to process our query."));
		m_httpRSafe->abort();
	}
	
	delete buffer;
}

void RapidTools::doneRSafe(bool)
{
	m_httpRSafe->deleteLater();
	pushDecode->setEnabled(true);
	m_httpRSafe = 0;
}

void RapidTools::extractRFLinks()
{
	QString url = lineURL->text();
	
	if(!url.startsWith("http://rapidshare.com/users/"))
	{
		QMessageBox::warning(this, "FatRat", tr("An invalid link has been encountered: %1").arg(url));
	}
	else if(!url.isEmpty())
	{
		textFolderContents->clear();
		pushExtract->setDisabled(true);
		
		m_httpRF = new QHttp("rapidshare.com", 80, this);
		m_bufRF = new QBuffer(m_httpRF);
		connect(m_httpRF, SIGNAL(done(bool)), this, SLOT(doneRF(bool)));
		m_httpRF->get(QUrl(url).path(), m_bufRF);
	}
}

void RapidTools::doneRF(bool error)
{
	if(!error)
	{
		const QByteArray& data = m_bufRF->data();
		QRegExp re("<a href=\"http://rapidshare.com/files/([^\"]+)");
		int pos = 0;
		
		while(true)
		{
			pos = re.indexIn(data, pos);
			if(pos < 0)
				break;
			
			textFolderContents->append( QString("http://rapidshare.com/files/") + re.cap(1));
			
			pos++;
		}
	}
	else
		QMessageBox::critical(this, "FatRat", tr("Server failed to process our query."));
	
	m_httpRF->deleteLater();
	m_httpRF = 0;
	m_bufRF = 0;
	pushExtract->setEnabled(true);
}

void RapidTools::downloadRFLinks()
{
	QString text = textFolderContents->toPlainText();
	if(!text.isEmpty())
	{
		MainWindow* wnd = (MainWindow*) getMainWindow();
		wnd->addTransfer(text);
	}
}

