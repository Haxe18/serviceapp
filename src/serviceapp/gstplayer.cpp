#include <sstream>

#include <lib/base/eerror.h>
#include "gstplayer.h"

const std::string GST_DOWNLOAD_BUFFER_PATH = "download_buffer_path";
const std::string GST_RING_BUFFER_MAXSIZE  = "ring_buffer_maxsize";
const std::string GST_BUFFER_SIZE          = "buffer_size";
const std::string GST_BUFFER_DURATION      = "buffer_duration";
const std::string GST_VIDEO_SINK           = "video_sink";
const std::string GST_AUDIO_SINK           = "audio_sink";
const std::string GST_AUDIO_TRACK_IDX      = "audio_id";
const std::string GST_SUBTITLE_ENABLED     = "subtitles_enabled";

GstPlayerOptions::GstPlayerOptions()
{
	settingMap[GST_DOWNLOAD_BUFFER_PATH] = SettingEntry("-p", "string");
	settingMap[GST_RING_BUFFER_MAXSIZE]  = SettingEntry("-r", "int");
	settingMap[GST_BUFFER_SIZE]          = SettingEntry("-s", 8*1024, "int");
	settingMap[GST_BUFFER_DURATION]      = SettingEntry("-d", 0, "int");
	settingMap[GST_VIDEO_SINK]           = SettingEntry("-v", "string");
	settingMap[GST_AUDIO_SINK]           = SettingEntry("-a", "string");
	settingMap[GST_AUDIO_TRACK_IDX]      = SettingEntry("-i", "int");
	settingMap[GST_SUBTITLE_ENABLED]     = SettingEntry("-e", true, "bool");
}

SettingMap &GstPlayerOptions::GetSettingMap()
{
	return settingMap;
}

int GstPlayerOptions::update(const std::string &key, const std::string &val)
{
	int ret = 0;
	if (settingMap.find(key) != settingMap.end())
	{
		SettingEntry &entry = settingMap[key];
		if (entry.getType() == "bool")
		{
			if (val == "1")
				entry.setValue(1);
			else if (val == "0")
				entry.setValue(0);
			else
			{
				eWarning("GstPlayerOptions::update - invalid value '%s' for '%s' setting, allowed values are 0|1", val.c_str(), key.c_str());
				ret = -2;
			}
		}
		else if (entry.getType() == "int")
		{
			char *endptr = NULL;
			int intval = -1;
			intval = strtol(val.c_str(), &endptr , 10);
			if (!*endptr && intval >= 0)
			{
				entry.setValue(intval);
			}
			else
			{
				eWarning("GstPlayerOptions::update - invalid value '%s' for '%s' setting, allowed values are >= 0", val.c_str(), key.c_str());
				ret = -2;
			}
		}
		else if (entry.getType() == "string")
		{
			if (val.empty())
			{
				eWarning("GstPlayerOptions::update - empty string for '%s' setting", key.c_str());
				ret = -2;
			}
			else
			{
				entry.setValue(val);
			}
		}
	}
	else
	{
		eWarning("GstPlayerOptions::update - not recognized setting '%s'", key.c_str());
		ret = -1;
	}
	return ret;
}

void GstPlayerOptions::print() const
{
	for (SettingIter it(settingMap.begin()); it != settingMap.end(); it++)
	{
		std::stringstream ss;
		ss << " %-30s = %s";
		if (it->first == GST_BUFFER_SIZE)
		{
			ss << "KB";
		}
		else if (it->first == GST_BUFFER_DURATION)
		{
			ss << "s";
		}
		eDebug(ss.str().c_str(), it->first.c_str(), it->second.toString().c_str());
	}
}

GstPlayer::GstPlayer(GstPlayerOptions& options): PlayerApp(STD_ERROR)
{
	mPlayerOptions = options;
	eDebug("GstPlayer::GstPlayer initializing with options:");
	mPlayerOptions.print();
}

std::vector<std::string> GstPlayer::buildCommand()
{
	std::vector<std::string> args;
	args.push_back("gstplayer_gst-1.0");
	args.push_back(mPath);
	for (std::map<std::string,std::string>::const_iterator i(mHeaders.begin()); i!=mHeaders.end(); i++)
	{
		args.push_back("-H");
		args.push_back(i->first + "=" + i->second);
	}
	for (SettingIter i(mPlayerOptions.GetSettingMap().begin()); i != mPlayerOptions.GetSettingMap().end(); i++)
	{
		if (!i->second.isSet())
		{
			continue;
		}
		if (i->second.getType() == "bool" && i->second.getValueInt())
		{
			args.push_back(i->second.getAppArg());
		}
		if (i->second.getType() == "int" || i->second.getType() == "string")
		{
			std::stringstream ss;
			ss << i->second.getAppArg();
			ss << " ";
			ss << i->second.getValue();
			args.push_back(ss.str());
		}
	}

	return args;
}
int GstPlayer::start(eMainloop* context)
{
	return processStart(context);
}

int GstPlayer::sendStop(){ return processSend(std::string("q\n"));}
int GstPlayer::sendForceStop(){ processKill(); return 0;}
int GstPlayer::sendPause(){ return processSend(std::string("p\n"));}
int GstPlayer::sendResume(){ return processSend(std::string("c\n"));}
int GstPlayer::sendUpdateLength(){ return processSend(std::string("l\n"));}
int GstPlayer::sendUpdatePosition(){ return processSend(std::string("j\n"));}
int GstPlayer::sendUpdateAudioTracksList(){ return processSend(std::string("al\n"));}
int GstPlayer::sendUpdateAudioTrackCurrent(){ return processSend(std::string("ac\n"));}

int GstPlayer::sendAudioSelectTrack(int trackId)
{
	std::stringstream sstm;
	sstm << "a" << trackId << std::endl;
	return processSend(sstm.str());
}

int GstPlayer::sendUpdateSubtitleTracksList(){ return processSend(std::string("sl\n"));}
int GstPlayer::sendUpdateSubtitleTrackCurrent(){ return processSend(std::string("sc\n"));}

int GstPlayer::sendSubtitleSelectTrack(int trackId)
{
	std::stringstream sstm;
	sstm << "s" << trackId << std::endl;
	return processSend(sstm.str());
}

int GstPlayer::sendSeekTo(int seconds)
{
	std::stringstream sstm;
	sstm << "gc" << seconds << std::endl;
	return processSend(sstm.str());
}

int GstPlayer::sendSeekRelative(int seconds)
{
	std::stringstream sstm;
	sstm << "kc" << seconds << std::endl;
	return processSend(sstm.str());
}

void GstPlayer::handleProcessStopped(int retval)
{
	recvStopped(0);
}

void GstPlayer::handleJsonOutput(cJSON *json)
{
	if (!json->child)
	{
		return;
	}
	const char *key = json->child->string;
	if (!key)
		return;
	cJSON* value = cJSON_GetObjectItem(json, key);

	if (!strcmp(key, "PLAYBACK_PLAY"))
	{
		if (!cJsonGetInt(value, "sts"))
		{
			recvStarted(0);
		}
	}
	else if (!strcmp(key, "PLAYBACK_INFO"))
	{
//		int isPlaying = cJsonGetInt(value, "isPlaying");
//		int isPaused = cJsonGetInt(value, "isPaused");
//		int isForwarding = cJsonGetInt(value, "isForwarding");
//		int isSeeking = cJsonGetInt(value, "isSeeking");
//		int isCreatingPhase = cJsonGetInt(value, "isCreatingPhase");
//		float backWard = cJsonGetDouble(value, "BackWard");
//		int slowMotion = cJsonGetInt(value, "SlowMotion");
//		int speed = cJsonGetInt(value, "Speed");
//		int avSync = cJsonGetInt(value, "AVSync");
//		int isAudio = cJsonGetInt(value, "isAudio");
//		int isVideo = cJsonGetInt(value, "isVideo");
//		int isSubtitle = cJsonGetInt(value, "isSubtitle");
	}
	else if (!strcmp(key, "v_c"))
	{
		videoStream v;
		v.id = cJsonGetInt(value, "id");
		v.description = cJsonGetStr(value, "e");
		v.language_code = cJsonGetStr(value, "n");
		v.width = cJsonGetInt(value, "w");
		v.height = cJsonGetInt(value, "h");
		v.framerate = cJsonGetInt(value, "f");
		
		// this would crash if somebody was using older version
		// of gstplayer where progressive was not passed
		cJSON *progressive = value ? cJSON_GetObjectItem(value, "p") : 0;
		if (progressive != NULL)
		{
			v.progressive = progressive->valueint;
		}
		recvVideoTrackCurrent(0, v);
	}
	else if (!strcmp(key, "a_s"))
	{
		if (!cJsonGetInt(value, "sts"))
		{
			int s = cJsonGetInt(value, "id");
			recvAudioTrackSelected(0, s);
			return;
		}
		recvAudioTrackSelected(1, -1);
	}
	else if (!strcmp(key, "a_c"))
	{
		audioStream a;
		a.id = cJsonGetInt(value, "id");
		a.description = cJsonGetStr(value, "e");
		a.language_code = cJsonGetStr(value, "n");
		recvAudioTrackCurrent(0, a);
	}
	else if (!strcmp(key, "a_l"))
	{
		std::vector<audioStream> streams;
		for (int i=0; i<cJSON_GetArraySize(value); i++)
		{
			cJSON *subitem=cJSON_GetArrayItem(value,i);
			audioStream a;
			a.id = cJsonGetInt(subitem, "id"); 
			a.description = cJsonGetStr(subitem, "e");
			a.language_code = cJsonGetStr(subitem, "n");
			streams.push_back(a);
		}
		recvAudioTracksList(0, streams);
	}
	else if (!strcmp(key, "s_s"))
	{
		if (!cJsonGetInt(value, "sts"))
		{
			int s = cJsonGetInt(value, "id");
			recvSubtitleTrackSelected(0, s);
			return;
		}
		recvSubtitleTrackSelected(1, -1);
	}
	else if (!strcmp(key, "s_c"))
	{
		subtitleStream s;
		s.id = cJsonGetInt(value, "id");
		s.description = cJsonGetStr(value, "e");
		s.language_code = cJsonGetStr(value, "n");
		recvSubtitleTrackCurrent(0, s);
	}
	else if (!strcmp(key, "s_l"))
	{
		std::vector<subtitleStream> streams;
		for (int i=0; i<cJSON_GetArraySize(value); i++)
		{
			cJSON *subitem=cJSON_GetArrayItem(value,i);
			subtitleStream s;
			s.id = cJsonGetInt(subitem, "id"); 
			s.description = cJsonGetStr(subitem, "e");
			s.language_code = cJsonGetStr(subitem, "n");
			streams.push_back(s);
		}
		recvSubtitleTracksList(0, streams);
	}
	else if (!strcmp(key, "PLAYBACK_SUBTITLE"))
	{
		subtitleMessage s;
		s.start_ms = cJsonGetInt(value, "start");
		s.duration_ms = cJsonGetInt(value, "duration");
		s.end_ms = s.start_ms + s.duration_ms;
		s.text = cJsonGetStr(value, "text");
		recvSubtitleMessage(s);
	}
	else if (!strcmp(key, "PLAYBACK_LENGTH"))
	{
		if (!cJsonGetInt(value, "sts"))
		{
			float l = cJsonGetDouble(value, "length");
			recvLength(0, l * 1000);
		}
	}
	else if (!strcmp(key, "J"))
	{
		int positionInMs = cJsonGetInt(value, "ms");
		recvPosition(0, positionInMs);
	}
	else if (!strcmp(key, "GST_ERROR"))
	{
		errorMessage e;
		e.message = cJsonGetStr(value, "msg");
		//e.code = cJsonGetInt(value, "code");
		recvErrorMessage(e);
	}
	else if (!strcmp(key, "GST_MISSING_PLUGIN"))
	{
		errorMessage e;
		e.message = "GStreamer plugin ";
		e.message += cJsonGetStr(value, "msg");
		e.message += " is not available!";
		recvErrorMessage(e);
	}
	else if (!strcmp(key, "PLAYBACK_STOP"))
	{
		if (!cJsonGetInt(value, "sts"))
		{
			//recvStopped(0);
			return;
		}
		//recvStopped(1);
	}
	else if (!strcmp(key, "PLAYBACK_CONTINUE"))
	{
		if (!cJsonGetInt(value, "sts"))
		{
			recvResumed(0);
			return;
		}
		recvResumed(1);
	}
	else if (!strcmp(key, "PLAYBACK_PAUSE"))
	{
		if (!cJsonGetInt(value, "sts"))
		{
			recvPaused(0);
			return;
		}
		recvPaused(1);
	}
	else if (!strcmp(key, "PLAYBACK_FASTFORWARD"))
	{
		if (!cJsonGetInt(value, "sts"))
		{
		}
	}
	else if (!strcmp(key, "PLAYBACK_SEEK_ABS"))
	{
		if (!cJsonGetInt(value, "sts"))
		{
			//FIXME
			recvSeekTo(0, 0);
			return;
		}
		recvSeekTo(1, 0);
	}
	else if (!strcmp(key, "PLAYBACK_SEEK"))
	{
		if (!cJsonGetInt(value, "sts"))
		{
			recvSeekRelative(0, 0);
			return;
		}
		recvSeekRelative(1, 0);
	}
	else
	{
		eDebug("GstPlayer::handleJsonOutput - unhandled key \"%s\"", key);
	}
}
