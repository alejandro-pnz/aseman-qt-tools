/*
    Copyright (C) 2017 Aseman Team
    http://aseman.co

    AsemanQtTools is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AsemanQtTools is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "asemantools.h"
#include "asemandevices.h"
#include "asemanqttools.h"

#include <QMetaMethod>
#include <QMetaObject>
#include <QCryptographicHash>
#include <QCoreApplication>
#include <QColor>
#include <QTimer>
#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QStringList>
#include <QTextDocument>
#ifndef Q_OS_IOS
#include <QProcess>
#endif
#include <QTimerEvent>
#include <QUuid>
#include <QMimeDatabase>
#include <QImageReader>
#include <QJsonDocument>

#ifdef QT_PURCHASING_LIB
#include <QInAppStore>
#endif

class AsemanToolsPrivate
{
public:
#ifdef QT_QML_LIB
    QHash<int, QJSValue> js_delay_call_timers;
#endif
};

AsemanTools::AsemanTools(QObject *parent) :
    QObject(parent)
{
    p = new AsemanToolsPrivate;
}

void AsemanTools::debug(const QVariant &var)
{
    qDebug() << var;
}

void AsemanTools::exportDocuments(const QString &destination)
{
    AsemanQtTools::exportDocuments(destination);
}

QDateTime AsemanTools::currentDate()
{
    return QDateTime::currentDateTime();
}

QString AsemanTools::dateToMSec(const QDateTime &dt)
{
    return QString::number(dt.toMSecsSinceEpoch());
}

QDateTime AsemanTools::mSecToDate(const QString &ms)
{
    return QDateTime::fromMSecsSinceEpoch(ms.toLongLong());
}

qint64 AsemanTools::dateToSec(const QDateTime &dt)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
    return dt.toSecsSinceEpoch();
#else
   return dt.toMSecsSinceEpoch()/1000;
#endif
}

QString AsemanTools::dateToString(const QDateTime &dt, const QString & format)
{
    if(format.isEmpty())
        return dt.toString();
    else
        return dt.toString(format);
}

QDate AsemanTools::dateAddDays(const QDate &date, int days)
{
    return date.addDays(days);
}

QStringList AsemanTools::toStringList(const QVariant &value)
{
    return value.toStringList();
}

QVariantMap AsemanTools::toVariantMap(const QVariant &value)
{
    return value.toMap();
}

QVariantList AsemanTools::toVariantList(const QVariant &value)
{
    return value.toList();
}

QVariant AsemanTools::jsonToVariant(const QString &json)
{
    return QJsonDocument::fromJson(json.toUtf8()).toVariant();
}

QString AsemanTools::variantToJson(QVariant value)
{
#ifdef QT_QML_LIB
    value = value.value<QJSValue>().toVariant();
#endif
    return QJsonDocument::fromVariant(value).toJson(QJsonDocument::Indented);
}

QString AsemanTools::byteArrayToString(const QVariant &value)
{
    return value.toByteArray();
}

QByteArray AsemanTools::compress(const QByteArray &data, int level)
{
    return qCompress(data, level);
}

QByteArray AsemanTools::uncompress(const QByteArray &data)
{
    return qUncompress(data);
}

QVariant AsemanTools::bytesToVariant(const QByteArray &_data)
{
    QVariant result;
    QByteArray data = _data;
    QDataStream stream(&data, QIODevice::ReadOnly);
    stream >> result;
    return result;
}

QByteArray AsemanTools::variantToBytes(const QVariant &var)
{
    QByteArray result;
    QDataStream stream(&result, QIODevice::WriteOnly);
    stream << var;
    return result;
}

QString AsemanTools::fileName(const QString &path)
{
    return QFileInfo(path).baseName();
}

QString AsemanTools::fileSuffix(const QString &_path)
{
    QString path = _path;
    if(path.left(AsemanDevices::localFilesPrePath().size()) == AsemanDevices::localFilesPrePath())
        path = path.mid(AsemanDevices::localFilesPrePath().size());

    QString result = QFileInfo(path).suffix().toLower();
    if(!result.isEmpty())
        return result;

    QMimeDatabase db;
    const QStringList &suffixes = db.mimeTypeForFile(path).suffixes();
    if(!suffixes.isEmpty())
        result = suffixes.first().toLower();

    return result;
}

QString AsemanTools::fileMime(const QString &path)
{
    QMimeDatabase db;
    return db.mimeTypeForFile(path).name();
}

QString AsemanTools::fileParent(const QString &path)
{
    return path.mid(0, path.lastIndexOf("/"));
}

QString AsemanTools::readText(const QString &path)
{
    QFile file(path);
    if( !file.open(QFile::ReadOnly) )
        return QString();

    QString res = QString::fromUtf8(file.readAll());
    return res;
}

bool AsemanTools::writeText(const QString &path, const QString &text)
{
    QFile file(path);
    if( !file.open(QFile::WriteOnly) )
        return false;

    file.write(text.toUtf8());
    file.close();
    return true;
}

bool AsemanTools::fileExists(const QString &path)
{
    return QFileInfo::exists(path);
}

QStringList AsemanTools::filesOf(const QString &path)
{
    return QDir(path).entryList(QDir::Files);
}

QSize AsemanTools::imageSize(const QString &_path)
{
    QString path = _path;
    if(path.left(AsemanDevices::localFilesPrePath().size()) == AsemanDevices::localFilesPrePath())
        path = path.mid(AsemanDevices::localFilesPrePath().size());

    QImageReader image(path);
    return image.size();
}

bool AsemanTools::writeFile(const QString &path, const QVariant &data, bool compress)
{
    QString dir = path.left(path.lastIndexOf("/")+1);
    QDir().mkpath(dir);

    QFile file(path);
    if(!file.open(QFile::WriteOnly))
        return false;

    QByteArray bytes;
    if(data.type() == QVariant::ByteArray)
        bytes = data.toByteArray();
    else
    {
        QDataStream stream(&bytes, QIODevice::WriteOnly);
        stream << data;
    }

    if(compress)
        bytes = qCompress(bytes);

    file.write(bytes);
    file.close();
    return true;
}

QByteArray AsemanTools::readFile(const QString &path, bool uncompress)
{
    QFile file(path);
    if(!file.open(QFile::ReadOnly))
        return QByteArray();

    QByteArray res = file.readAll();
    file.close();

    if(uncompress)
        res = qUncompress(res);

    return res;
}

QString AsemanTools::className(QObject *obj)
{
    QString result;
    if(obj)
        result = obj->metaObject()->className();

    int idx = result.indexOf("_QMLTYPE");
    if(idx >= 0)
        result = result.left(idx+8);
    return result;
}

QStringList AsemanTools::stringLinks(const QString &str)
{
    QStringList links;
    QRegExp links_rxp("((?:(?:\\w\\S*\\/\\S*|\\/\\S+|\\:\\/)(?:\\/\\S*\\w|\\w))|(?:\\w+\\.(?:com|org|co|net)))");
    int pos = 0;
    while ((pos = links_rxp.indexIn(str, pos)) != -1)
    {
        QString link = links_rxp.cap(1);
        if(link.indexOf(QRegExp("\\w+\\:\\/\\/")) == -1)
            link = "http://" + link;

        links << link;
        pos += links_rxp.matchedLength();
    }

    return links;
}

QUrl AsemanTools::stringToUrl(const QString &path)
{
    return QUrl(path);
}

QString AsemanTools::urlToLocalPath(const QUrl &url)
{
    QString res = url.toLocalFile();
    if(res.isEmpty())
        res = url.toString();

    int idx1 = res.indexOf(":/");
    if(idx1 != -1)
    {
        int idx2 = res.lastIndexOf(":/");
        if(idx2 != -1)
            res.remove(idx1, idx2-idx1);
    }

    if(res.left(AsemanDevices::localFilesPrePath().size()) == AsemanDevices::localFilesPrePath())
        res = res.mid(AsemanDevices::localFilesPrePath().size());
    if(res.left(5) == "qrc:/")
        res = res.mid(3);

    return res;
}

QString AsemanTools::qtVersion()
{
    return qVersion();
}

QString AsemanTools::aboutAseman()
{
    return tr("Aseman is a not-for-profit research and software development team launched in February 2014 focusing on development of products, technologies and solutions in order to publish them as open-source projects accessible to all people in the universe. Currently, we are focusing on design and development of software applications and tools which have direct connection with end users.") + "\n\n" +
           tr("By enabling innovative projects and distributing software to millions of users globally, the lab is working to accelerate the growth of high-impact open source software projects and promote an open source culture of accessibility and increased productivity around the world. The lab partners with industry leaders and policy makers to bring open source technologies to new sectors, including education, health and government.");
}

void AsemanTools::deleteItemDelay(QObject *o, int ms)
{
    QTimer::singleShot( ms, o, SLOT(deleteLater()) );
}

qreal AsemanTools::colorHue(const QColor &clr)
{
    return clr.hue()/359.0;
}

qreal AsemanTools::colorLightness(const QColor &clr)
{
    return clr.lightness()/255.0;
}

qreal AsemanTools::colorSaturation(const QColor &clr)
{
    return clr.saturation()/255.0;
}

void AsemanTools::mkDir(const QString &pt)
{
    QString path = pt;
    if(path.left(AsemanDevices::localFilesPrePath().size()) == AsemanDevices::localFilesPrePath())
        path = path.mid(AsemanDevices::localFilesPrePath().size());

    QDir().mkpath(path);
}

QVariantMap AsemanTools::colorHsl(const QColor &clr)
{
    QVariantMap res;
    res["hue"] = colorHue(clr);
    res["lightness"] = colorLightness(clr);
    res["saturation"] = colorSaturation(clr);

    return res;
}

bool AsemanTools::createVideoThumbnail(const QString &video, const QString &output, QString ffmpegPath)
{
#ifdef Q_OS_IOS
    Q_UNUSED(video)
    Q_UNUSED(output)
    Q_UNUSED(ffmpegPath)
    return false;
#else
    if(ffmpegPath.isEmpty())
#ifdef Q_OS_WIN
        ffmpegPath = QCoreApplication::applicationDirPath() + "/ffmpeg.exe";
#else
#ifdef Q_OS_MAC
        ffmpegPath = QCoreApplication::applicationDirPath() + "/ffmpeg";
#else
    {
        if(QFileInfo::exists("/usr/bin/avconv"))
            ffmpegPath = "/usr/bin/avconv";
        else
            ffmpegPath = "ffmpeg";
    }
#endif
#endif

    QStringList args;
    args << "-itsoffset";
    args << "-4";
    args << "-i";
    args << video;
    args << "-vcodec";
    args << "mjpeg";
    args << "-vframes";
    args << "1";
    args << "-an";
    args << "-f";
    args << "rawvideo";
    args << output;
    args << "-y";

    QProcess prc;
    prc.start(ffmpegPath, args);
    prc.waitForStarted();
    prc.waitForFinished();

    return prc.exitCode() == 0;
#endif
}

QString AsemanTools::translateNumbers(QString input)
{
    input.replace("0", AsemanTools::tr("0"));
    input.replace("1", AsemanTools::tr("1"));
    input.replace("2", AsemanTools::tr("2"));
    input.replace("3", AsemanTools::tr("3"));
    input.replace("4", AsemanTools::tr("4"));
    input.replace("5", AsemanTools::tr("5"));
    input.replace("6", AsemanTools::tr("6"));
    input.replace("7", AsemanTools::tr("7"));
    input.replace("8", AsemanTools::tr("8"));
    input.replace("9", AsemanTools::tr("9"));
    return input;
}

QString AsemanTools::trNums(QString input)
{
    return AsemanTools::translateNumbers(input);
}

QString AsemanTools::passToMd5(const QString &pass)
{
    if( pass.isEmpty() )
        return QString();

    return QCryptographicHash::hash( pass.toUtf8(), QCryptographicHash::Md5 ).toHex();
}

QString AsemanTools::md5(const QString &str)
{
    return QCryptographicHash::hash( str.toUtf8(), QCryptographicHash::Md5 ).toHex();
}

QString AsemanTools::createUuid()
{
    return QUuid::createUuid().toString();
}

QString AsemanTools::stringRemove(QString str, const QString &text, bool regExp)
{
    if(regExp)
        return str.remove( QRegExp(text) );
    else
        return str.remove(text);
}

QString AsemanTools::htmlToPlaintText(const QString &html)
{
    QTextDocument doc;
    doc.setHtml(html);
    return doc.toPlainText();
}

void AsemanTools::copyDirectory(const QString &_src, const QString &_dst)
{
    QString src = _src;
    if(src.left(AsemanDevices::localFilesPrePath().size()) == AsemanDevices::localFilesPrePath())
        src = src.mid(AsemanDevices::localFilesPrePath().size());
    QString dst = _dst;
    if(dst.left(AsemanDevices::localFilesPrePath().size()) == AsemanDevices::localFilesPrePath())
        dst = dst.mid(AsemanDevices::localFilesPrePath().size());

    QDir().mkpath(dst);

    const QStringList & dirs = QDir(src).entryList(QDir::Dirs|QDir::NoDotAndDotDot);
    for(const QString &d: dirs)
        copyDirectory(src+"/"+d, dst+"/"+d);

    const QStringList & files = QDir(src).entryList(QDir::Files);
    for(const QString &f: files)
        QFile::copy(src+"/"+f, dst+"/"+f);
}

bool AsemanTools::copy(const QString &_src, const QString &_dst)
{
    QString src = _src;
    if(src.left(AsemanDevices::localFilesPrePath().size()) == AsemanDevices::localFilesPrePath())
        src = src.mid(AsemanDevices::localFilesPrePath().size());
    QString dst = _dst;
    if(dst.left(AsemanDevices::localFilesPrePath().size()) == AsemanDevices::localFilesPrePath())
        dst = dst.mid(AsemanDevices::localFilesPrePath().size());

    return QFile::copy(src, dst);
}

void AsemanTools::deleteFile(const QString &pt)
{
    QString path = pt;
    if(path.left(AsemanDevices::localFilesPrePath().size()) == AsemanDevices::localFilesPrePath())
        path = path.mid(AsemanDevices::localFilesPrePath().size());

    QFile::remove(path);
}

void AsemanTools::clearDirectory(const QString &pt)
{
    QString path = pt;
    if(path.left(AsemanDevices::localFilesPrePath().size()) == AsemanDevices::localFilesPrePath())
        path = path.mid(AsemanDevices::localFilesPrePath().size());

    const QStringList & files = QDir(path).entryList(QDir::Files);
    for(const QString &f: files)
        deleteFile(path+"/"+f);
}

void AsemanTools::setProperty(QObject *obj, const QString &property, const QVariant &v)
{
    if( !obj || property.isEmpty() )
        return;

    obj->setProperty( property.toUtf8(), v );
}

QVariant AsemanTools::property(QObject *obj, const QString &property)
{
    if( !obj || property.isEmpty() )
        return QVariant();

    return obj->property(property.toUtf8());
}

Qt::LayoutDirection AsemanTools::directionOf(const QString &str)
{
    Qt::LayoutDirection res = Qt::LeftToRight;
    if( str.isEmpty() )
        return res;

    int ltr = 0;
    int rtl = 0;

    for(const QChar &ch: str)
    {
        QChar::Direction dir = ch.direction();
        switch( static_cast<int>(dir) )
        {
        case QChar::DirL:
        case QChar::DirLRE:
        case QChar::DirLRO:
        case QChar::DirEN:
            ltr++;
            break;

        case QChar::DirR:
        case QChar::DirRLE:
        case QChar::DirRLO:
        case QChar::DirAL:
            rtl++;
            break;
        }
    }

    if( ltr >= rtl )
        res = Qt::LeftToRight;
    else
        res = Qt::RightToLeft;

    return res;
}

QVariant AsemanTools::call(QObject *obj, const QString &member, Qt::ConnectionType ctype, const QVariant &v0, const QVariant &v1, const QVariant &v2, const QVariant &v3, const QVariant &v4, const QVariant &v5, const QVariant &v6, const QVariant &v7, const QVariant &v8, const QVariant &v9)
{
    const QMetaObject *meta_obj = obj->metaObject();
    QMetaMethod meta_method;
    for( int i=0; i<meta_obj->methodCount(); i++ )
    {
        QMetaMethod mtd = meta_obj->method(i);
        if( mtd.name() == member )
            meta_method = mtd;
    }
    if( !meta_method.isValid() )
        return QVariant();

    QList<QByteArray> param_types = meta_method.parameterTypes();
    QList<QByteArray> param_names = meta_method.parameterNames();

    QString ret_type = meta_method.typeName();
    QList< QPair<QString,QString> > m_args;
    for( int i=0 ; i<param_types.count() ; i++ )
        m_args << QPair<QString,QString>( param_types.at(i) , param_names.at(i) );

    QVariantList vals;
        vals << v0 << v1 << v2 << v3 << v4 << v5 << v6 << v7 << v8 << v9;

    QVariantList tr_vals;

    QList< QPair<QString,const void*> > args;
    for( int i=0 ; i<vals.count() ; i++ )
    {
        if( i<m_args.count() )
        {
            QString type = m_args.at(i).first;

            if( type != vals.at(i).typeName() )
            {
                if( !vals[i].canConvert( QVariant::nameToType(type.toLatin1()) ) )
                    ;
                else
                    vals[i].convert( QVariant::nameToType(type.toLatin1()) );
            }

            args << QPair<QString,const void*>( type.toLatin1() , vals.at(i).data() );
            tr_vals << vals[i];
        }
        else
        {
            args << QPair<QString,const void*>( vals.at(i).typeName() , vals.at(i).data() );
        }
    }

    int type = QMetaType::type(ret_type.toLatin1());
    void *res = QMetaType::create( type );
    bool is_pointer = ret_type.contains('*');

    bool done;
    switch( static_cast<int>(ctype) )
    {
    case Qt::QueuedConnection:
        done = QMetaObject::invokeMethod( obj , member.toLatin1() , Qt::QueuedConnection ,
                                  QGenericArgument( args.at(0).first.toLatin1() , args.at(0).second ) ,
                                  QGenericArgument( args.at(1).first.toLatin1() , args.at(1).second ) ,
                                  QGenericArgument( args.at(2).first.toLatin1() , args.at(2).second ) ,
                                  QGenericArgument( args.at(3).first.toLatin1() , args.at(3).second ) ,
                                  QGenericArgument( args.at(4).first.toLatin1() , args.at(4).second ) ,
                                  QGenericArgument( args.at(5).first.toLatin1() , args.at(5).second ) ,
                                  QGenericArgument( args.at(6).first.toLatin1() , args.at(6).second ) ,
                                  QGenericArgument( args.at(7).first.toLatin1() , args.at(7).second ) ,
                                  QGenericArgument( args.at(8).first.toLatin1() , args.at(8).second ) ,
                                  QGenericArgument( args.at(9).first.toLatin1() , args.at(9).second ) );
        return QVariant();
        break;

    default:
        done = QMetaObject::invokeMethod( obj , member.toLatin1() , ctype, QGenericReturnArgument( ret_type.toLatin1() , (is_pointer)? &res : res ) ,
                                  QGenericArgument( args.at(0).first.toLatin1() , args.at(0).second ) ,
                                  QGenericArgument( args.at(1).first.toLatin1() , args.at(1).second ) ,
                                  QGenericArgument( args.at(2).first.toLatin1() , args.at(2).second ) ,
                                  QGenericArgument( args.at(3).first.toLatin1() , args.at(3).second ) ,
                                  QGenericArgument( args.at(4).first.toLatin1() , args.at(4).second ) ,
                                  QGenericArgument( args.at(5).first.toLatin1() , args.at(5).second ) ,
                                  QGenericArgument( args.at(6).first.toLatin1() , args.at(6).second ) ,
                                  QGenericArgument( args.at(7).first.toLatin1() , args.at(7).second ) ,
                                  QGenericArgument( args.at(8).first.toLatin1() , args.at(8).second ) ,
                                  QGenericArgument( args.at(9).first.toLatin1() , args.at(9).second ) );
        break;
    }

    QVariant result;
    if( !done )
    {
        QMetaType::destroy(type, res);
        return result;
    }

    if( type == QMetaType::Void )
        result = QVariant();
    else
    if( is_pointer )
        result = QVariant( type , &res );
    else
        result = QVariant( type , res );

    if( type == QMetaType::type("QVariant") )
        return result.value<QVariant>();
    else
        return result;
}

#ifdef QT_QML_LIB
void AsemanTools::jsDelayCall(int ms, const QJSValue &value)
{
    p->js_delay_call_timers[ startTimer(ms) ] = value;
}
#endif

void AsemanTools::setInAppStoreProperty(QObject *store, const QString &propertyName, const QString &value)
{
#ifdef QT_PURCHASING_LIB
    if(!store)
        return;
    QList<QObject*> childs = store->children();
    childs.prepend(store);
    for(QObject *obj: childs)
        if( qobject_cast<QInAppStore*>(obj) )
            qobject_cast<QInAppStore*>(obj)->setPlatformProperty(propertyName, value);
#else
    Q_UNUSED(store)
    Q_UNUSED(propertyName)
    Q_UNUSED(value)
#endif
}

void AsemanTools::timerEvent(QTimerEvent *e)
{
#ifdef QT_QML_LIB
    if(p->js_delay_call_timers.contains(e->timerId()))
    {
        p->js_delay_call_timers.take(e->timerId()).call();
    }
    else
#endif
        QObject::timerEvent(e);
}

AsemanTools::~AsemanTools()
{
    delete p;
}
