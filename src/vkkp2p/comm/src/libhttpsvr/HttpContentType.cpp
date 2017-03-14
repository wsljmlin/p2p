
#include "HttpContentType.h"

//".*"="application/octet-stream" 
//".acp"="audio/x-mei-aac" 
//".aif"="audio/aiff" 
//".aifc"="audio/aiff" 
//".aiff"="audio/aiff" 
//".anv"="application/x-anv" 
//".asf"="video/x-ms-asf" 
//".asx"="video/x-ms-asf" 
//".au"="audio/basic" 
//".avi"="video/avi" 
//
//".ivf"="video/x-ivf" 
//".la1"="audio/x-liquid-file" 
//".lavs"="audio/x-liquid-secure" 
//".m1v"="video/x-mpeg" 
//".m2v"="video/x-mpeg" 
//".m3u"="audio/mpegurl" 
//".m4e"="video/mpeg4" 
//
//".mi"="application/x-mi" 
//".mid"="audio/mid" 
//".midi"="audio/mid" 
//".mil"="application/x-mil" 
//".mnd"="audio/x-musicnet-download" 
//".mns"="audio/x-musicnet-stream" 
//
//".movie"="video/x-sgi-movie" 
//".mp1"="audio/mp1" 
//".mp2"="audio/mp2" 
//".mp2v"="video/mpeg" 
//".mp3"="audio/mp3" 
//".mp4"="video/mpeg4" 
//".mpa"="video/x-mpg" 
//".mpd"="application/vnd.ms-project" 
//".mpe"="video/x-mpeg" 
//".mpeg"="video/mpg" 
//".mpg"="video/mpg" 
//".mpga"="audio/rn-mpeg" 
//
//".mps"="video/x-mpeg" 
//
//".mpv"="video/mpg" 
//".mpv2"="video/mpeg" 
//
//
//
//".pls"="audio/scpls" 
//".plt"="application/x-plt" 
//
//".ra"="audio/vnd.rn-realaudio" 
//".ram"="audio/x-pn-realaudio" 
//".ras"="application/x-ras" 
//".rat"="application/rat-file" 
//".rec"="application/vnd.rn-recording" 
//".red"="application/x-red" 
//".rgb"="application/x-rgb" 
//".rjs"="application/vnd.rn-realsystem-rjs" 
//".rjt"="application/vnd.rn-realsystem-rjt" 
//".rlc"="application/x-rlc" 
//".rle"="application/x-rle" 
//".rm"="application/vnd.rn-realmedia" 
//".rmf"="application/vnd.adobe.rmf" 
//".rmi"="audio/mid" 
//".rmj"="application/vnd.rn-realsystem-rmj" 
//".rmm"="audio/x-pn-realaudio" 
//".rmp"="application/vnd.rn-rn_music_package" 
//".rms"="application/vnd.rn-realmedia-secure" 
//".rmvb"="application/vnd.rn-realmedia-vbr" 
//".rmx"="application/vnd.rn-realsystem-rmx" 
//".rnx"="application/vnd.rn-realplayer" 
//".rp"="image/vnd.rn-realpix" 
//".rpm"="audio/x-pn-realaudio-plugin" 
//".rv"="video/vnd.rn-realvideo" 
//".snd"="audio/basic" 
//".torrent"="application/x-bittorrent" 
//".txt"="text/plain" 
//".vxml"="text/xml" 
//".wav"="audio/wav" 
//".wax"="audio/x-ms-wax" 
//".wm"="video/x-ms-wm" 
//".wma"="audio/x-ms-wma" 
//".wmv"="video/x-ms-wmv" 
//".wmx"="video/x-ms-wmx" 
//".wvx"="video/x-ms-wvx" 
//".xml"="text/xml" 



HttpContentType::HttpContentType(void)
{
	m_ct_map[".*"]="application/octet-stream" ;
	m_ct_map[".acp"]="audio/x-mei-aac" ;
	m_ct_map[".aif"]="audio/aiff" ;
	m_ct_map[".aifc"]="audio/aiff" ;
	m_ct_map[".aiff"]="audio/aiff" ;
	m_ct_map[".anv"]="application/x-anv" ;
	m_ct_map[".asf"]="video/x-ms-asf" ;
	m_ct_map[".asx"]="video/x-ms-asf" ;
	m_ct_map[".au"]="audio/basic" ;
	m_ct_map[".avi"]="video/avi" ;

	m_ct_map[".ivf"]="video/x-ivf"; 
	m_ct_map[".la1"]="audio/x-liquid-file" ;
	m_ct_map[".lavs"]="audio/x-liquid-secure" ;
	m_ct_map[".m1v"]="video/x-mpeg" ;
	m_ct_map[".m2v"]="video/x-mpeg" ;
	m_ct_map[".m3u"]="audio/mpegurl" ;
	m_ct_map[".m4e"]="video/mpeg4" ;

	m_ct_map[".mi"]="application/x-mi" ;
	m_ct_map[".mid"]="audio/mid" ;
	m_ct_map[".midi"]="audio/mid" ;
	m_ct_map[".mil"]="application/x-mil" ;
	m_ct_map[".mnd"]="audio/x-musicnet-download" ;
	m_ct_map[".mns"]="audio/x-musicnet-stream" ;

	m_ct_map[".movie"]="video/x-sgi-movie" ;
	m_ct_map[".mp1"]="audio/mp1" ;
	m_ct_map[".mp2"]="audio/mp2" ;
	m_ct_map[".mp2v"]="video/mpeg" ;
	m_ct_map[".mp3"]="audio/mp3" ;
	m_ct_map[".mp4"]="video/mpeg4" ;
	m_ct_map[".mpa"]="video/x-mpg" ;
	m_ct_map[".mpd"]="application/vnd.ms-project" ;
	m_ct_map[".mpe"]="video/x-mpeg" ;
	m_ct_map[".mpeg"]="video/mpg" ;
	m_ct_map[".mpg"]="video/mpg" ;
	m_ct_map[".mpga"]="audio/rn-mpeg" ;

	m_ct_map[".mps"]="video/x-mpeg" ;

	m_ct_map[".mpv"]="video/mpg" ;
	m_ct_map[".mpv2"]="video/mpeg" ;



	m_ct_map[".pls"]="audio/scpls" ;
	m_ct_map[".plt"]="application/x-plt" ;

	m_ct_map[".ra"]="audio/vnd.rn-realaudio" ;
	m_ct_map[".ram"]="audio/x-pn-realaudio" ;
	m_ct_map[".ras"]="application/x-ras" ;
	m_ct_map[".rat"]="application/rat-file" ;
	m_ct_map[".rec"]="application/vnd.rn-recording" ;
	m_ct_map[".red"]="application/x-red" ;
	m_ct_map[".rgb"]="application/x-rgb" ;
	m_ct_map[".rjs"]="application/vnd.rn-realsystem-rjs" ;
	m_ct_map[".rjt"]="application/vnd.rn-realsystem-rjt" ;
	m_ct_map[".rlc"]="application/x-rlc" ;
	m_ct_map[".rle"]="application/x-rle" ;
	m_ct_map[".rm"]="application/vnd.rn-realmedia" ;
	m_ct_map[".rmf"]="application/vnd.adobe.rmf" ;
	m_ct_map[".rmi"]="audio/mid" ;
	m_ct_map[".rmj"]="application/vnd.rn-realsystem-rmj" ;
	m_ct_map[".rmm"]="audio/x-pn-realaudio" ;
	m_ct_map[".rmp"]="application/vnd.rn-rn_music_package" ;
	m_ct_map[".rms"]="application/vnd.rn-realmedia-secure" ;
	m_ct_map[".rmvb"]="application/vnd.rn-realmedia-vbr" ;
	m_ct_map[".rmx"]="application/vnd.rn-realsystem-rmx" ;
	m_ct_map[".rnx"]="application/vnd.rn-realplayer" ;
	m_ct_map[".rp"]="image/vnd.rn-realpix" ;
	m_ct_map[".rpm"]="audio/x-pn-realaudio-plugin" ;
	m_ct_map[".rv"]="video/vnd.rn-realvideo" ;
	m_ct_map[".snd"]="audio/basic" ;
	m_ct_map[".torrent"]="application/x-bittorrent" ;
	m_ct_map[".txt"]="text/plain" ;
	m_ct_map[".vxml"]="text/xml" ;
	m_ct_map[".wav"]="audio/wav" ;
	m_ct_map[".wax"]="audio/x-ms-wax" ;
	m_ct_map[".wm"]="video/x-ms-wm" ;
	m_ct_map[".wma"]="audio/x-ms-wma" ;
	m_ct_map[".wmv"]="video/x-ms-wmv" ;
	m_ct_map[".wmx"]="video/x-ms-wmx" ;
	m_ct_map[".wvx"]="video/x-ms-wvx" ;
	m_ct_map[".xml"]="text/xml" ;

}
HttpContentType::~HttpContentType(void)
{
	m_ct_map.clear();
}
string HttpContentType::get_ct_name(const string& key)
{
	string str;
	if(key.empty())
		return "application/octet-stream";
	if(key.at(0)!='.')
	{
		str = ".";
		str += key;
	}
	else
		str = key.c_str();
	strlwr((char*)str.c_str());
	
	map<string,string>::iterator it = m_ct_map.find(str);
	if(it!=m_ct_map.end())
		return it->second;
	else
		return "application/octet-stream";
}
string HttpContentType::operator [](const string& key)
{
	return get_ct_name(key);
}

