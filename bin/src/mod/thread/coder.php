<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

class ModThreadCoder{
	var $pcodecount=0;
	var $codehtml=array();
	var $skipaidlist=array();
	var $smilies=array(),$smileytypes=array();
	var $resources;

	function ModThreadCoder(){
		$this->resources=<<<EOD
<style type="text/css">
.quote { overflow: hidden; margin: 10px 0; padding-left: 16px; background: url({IMGDIR}/qa.gif) no-repeat 0 0; color: {MIDTEXT}; }
	.quote blockquote { display: inline; margin: 0; padding-right: 16px; background: url({IMGDIR}/qz.gif) no-repeat 100% 100%; }
	.m_c .quote { width: 585px; }
.blockcode { overflow: hidden; margin: 10px 0; padding: 5px 10px; background: #F7F7F7; color: {MIDTEXT}; }
	.blockcode code { font-family: Monaco, Consolas, "Lucida Console", "Courier New", serif; font-size: 12px; line-height: 1.8em; }
	* html .blockcode code { font-family: "Courier New", serif; }
</style>
<script type="text/javascript">
var BROWSER = {};
var USERAGENT = navigator.userAgent.toLowerCase();
browserVersion({'ie':'msie','firefox':'','chrome':'','opera':'','safari':'','mozilla':'','webkit':'','maxthon':'','qq':'qqbrowser'});
if(BROWSER.safari) {
	BROWSER.firefox = true;
}
function browserVersion(types) {
	var other = 1;
	for(i in types) {
		var v = types[i] ? types[i] : i;
		if(USERAGENT.indexOf(v) != -1) {
			var re = new RegExp(v + '(\\/|\\s)([\\d\\.]+)', 'ig');
			var matches = re.exec(USERAGENT);
			var ver = matches != null ? matches[2] : 0;
			other = ver !== 0 && v != 'mozilla' ? 0 : other;
		}else {
			var ver = 0;
		}
		eval('BROWSER.' + i + '= ver');
	}
	BROWSER.other = other;
}
function AC_FL_RunContent() {
	var str = '';
	var ret = AC_GetArgs(arguments, "clsid:d27cdb6e-ae6d-11cf-96b8-444553540000", "application/x-shockwave-flash");
	if(BROWSER.ie && !BROWSER.opera) {
		str += '<object ';
		for (var i in ret.objAttrs) {
			str += i + '="' + ret.objAttrs[i] + '" ';
		}
		str += '>';
		for (var i in ret.params) {
			str += '<param name="' + i + '" value="' + ret.params[i] + '" /> ';
		}
		str += '</object>';
	} else {
		str += '<embed ';
		for (var i in ret.embedAttrs) {
			str += i + '="' + ret.embedAttrs[i] + '" ';
		}
		str += '></embed>';
	}
	return str;
}

function AC_GetArgs(args, classid, mimeType) {
	var ret = new Object();
	ret.embedAttrs = new Object();
	ret.params = new Object();
	ret.objAttrs = new Object();
	for (var i = 0; i < args.length; i = i + 2){
		var currArg = args[i].toLowerCase();
		switch (currArg){
			case "classid":break;
			case "pluginspage":ret.embedAttrs[args[i]] = 'http://www.macromedia.com/go/getflashplayer';break;
			case "src":ret.embedAttrs[args[i]] = args[i+1];ret.params["movie"] = args[i+1];break;
			case "codebase":ret.objAttrs[args[i]] = 'http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=9,0,0,0';break;
			case "onafterupdate":case "onbeforeupdate":case "onblur":case "oncellchange":case "onclick":case "ondblclick":case "ondrag":case "ondragend":
			case "ondragenter":case "ondragleave":case "ondragover":case "ondrop":case "onfinish":case "onfocus":case "onhelp":case "onmousedown":
			case "onmouseup":case "onmouseover":case "onmousemove":case "onmouseout":case "onkeypress":case "onkeydown":case "onkeyup":case "onload":
			case "onlosecapture":case "onpropertychange":case "onreadystatechange":case "onrowsdelete":case "onrowenter":case "onrowexit":case "onrowsinserted":case "onstart":
			case "onscroll":case "onbeforeeditfocus":case "onactivate":case "onbeforedeactivate":case "ondeactivate":case "type":
			case "id":ret.objAttrs[args[i]] = args[i+1];break;
			case "width":case "height":case "align":case "vspace": case "hspace":case "class":case "title":case "accesskey":case "name":
			case "tabindex":ret.embedAttrs[args[i]] = ret.objAttrs[args[i]] = args[i+1];break;
			default:ret.embedAttrs[args[i]] = ret.params[args[i]] = args[i+1];
		}
	}
	ret.objAttrs["classid"] = classid;
	if(mimeType) {
		ret.embedAttrs["type"] = mimeType;
	}
	return ret;
}
function preg_replace(search, replace, str, regswitch) {
	var regswitch = !regswitch ? 'ig' : regswitch;
	var len = search.length;
	for(var i = 0; i < len; i++) {
		re = new RegExp(search[i], regswitch);
		str = str.replace(re, typeof replace == 'string' ? replace : (replace[i] ? replace[i] : replace[0]));
	}
	return str;
}

function htmlspecialchars(str) {
	return preg_replace(['&', '<', '>', '"'], ['&amp;', '&lt;', '&gt;', '&quot;'], str);
}
</script>
EOD;
		$this->resources=str_replace(array('{WEBURL}','{IMGDIR}','{MIDTEXT}'),array(WEB_URL,WEB_URL.'static/image/common',WEB_MIDTEXT),$this->resources);
		$this->get_smilies();
	}
	function coder($tid,$uid,$authorid,$message){
		$this->tid=$tid;
		$this->uid=$uid;
		$field=MOD('thread.field')->get($tid);
		return $this->resources.$this->decoder($message, $field['smileyoff'], $field['bbcodeoff'], $field['htmlon'] & 1, $field['allowsmilies'], $field['allowbbcode'], ($field['allowimgcode'] ? 1 : 0), $field['allowhtml'], ($field['jammer'] && $authorid != $uid ? 1 : 0), 0, $authorid, $field['allowmediacode']);
	}
	function decoder($message, $smileyoff, $bbcodeoff, $htmlon = 0, $allowsmilies = 1, $allowbbcode = 1, $allowimgcode = 1, $allowhtml = 0, $jammer = 0, $parsetype = '0', $authorid = '0', $allowmediacode = '0', $pid = 0, $lazyload = 0) {
		global $_G;

		static $authorreplyexist;

		if($parsetype != 1 && !$bbcodeoff && $allowbbcode && (strpos($message, '[/code]') || strpos($message, '[/CODE]')) !== FALSE) {
			$message = preg_replace("/\s?\[code\](.+?)\[\/code\]\s?/ies", "\$this->codedisp('\\1')", $message);
		}

		$msglower = strtolower($message);

		$htmlon = $htmlon && $allowhtml ? 1 : 0;

		if(!$htmlon) {
			$message = shtmlspecialchars($message);
		}

		if(!$smileyoff && $allowsmilies) {
			$message = $this->parsesmiles($message);
		}

		if(WEB_ALLOWATTACHURL && strpos($msglower, 'attach://') !== FALSE) {
			$message = preg_replace("/attach:\/\/(\d+)\.?(\w*)/ie", "\$this->parseattachurl('\\1', '\\2')", $message);
		}

		if($allowbbcode) {
			if(strpos($msglower, 'ed2k://') !== FALSE) {
				$message = preg_replace("/ed2k:\/\/(.+?)\//e", "\$this->parseed2k('\\1')", $message);
			}
		}

		if(!$bbcodeoff && $allowbbcode) {
			if(strpos($msglower, '[/url]') !== FALSE) {
				$message = preg_replace("/\[url(=((https?|ftp|gopher|news|telnet|rtsp|mms|callto|bctp|thunder|qqdl|synacast){1}:\/\/|www\.|mailto:)?([^\r\n\[\"']+?))?\](.+?)\[\/url\]/ies", "\$this->parseurl('\\1', '\\5', '\\2')", $message);
			}
			if(strpos($msglower, '[/email]') !== FALSE) {
				$message = preg_replace("/\[email(=([a-z0-9\-_.+]+)@([a-z0-9\-_]+[.][a-z0-9\-_.]+))?\](.+?)\[\/email\]/ies", "\$this->parseemail('\\1', '\\4')", $message);
			}

			$nest = 0;
			while(strpos($msglower, '[table') !== FALSE && strpos($msglower, '[/table]') !== FALSE){
				$message = preg_replace("/\[table(?:=(\d{1,4}%?)(?:,([\(\)%,#\w ]+))?)?\]\s*(.+?)\s*\[\/table\]/ies", "\$this->parsetable('\\1', '\\2', '\\3')", $message);
				if(++$nest > 4) break;
			}

			$message = str_replace(array(
				'[/color]', '[/backcolor]', '[/size]', '[/font]', '[/align]', '[b]', '[/b]', '[s]', '[/s]', '[hr]', '[/p]',
				'[i=s]', '[i]', '[/i]', '[u]', '[/u]', '[list]', '[list=1]', '[list=a]',
				'[list=A]', "\r\n[*]", '[*]', '[/list]', '[indent]', '[/indent]', '[/float]'
				), array(
				'</font>', '</font>', '</font>', '</font>', '</p>', '<strong>', '</strong>', '<strike>', '</strike>', '<hr class="l" />', '</p>', '<i class="pstatus">', '<i>',
				'</i>', '<u>', '</u>', '<ul>', '<ul type="1" class="litype_1">', '<ul type="a" class="litype_2">',
				'<ul type="A" class="litype_3">', '<li>', '<li>', '</ul>', '<blockquote>', '</blockquote>', '</span>'
				), preg_replace(array(
				"/\[color=([#\w]+?)\]/i",
				"/\[color=(rgb\([\d\s,]+?\))\]/i",
				"/\[backcolor=([#\w]+?)\]/i",
				"/\[backcolor=(rgb\([\d\s,]+?\))\]/i",
				"/\[size=(\d{1,2}?)\]/i",
				"/\[size=(\d{1,2}(\.\d{1,2}+)?(px|pt)+?)\]/i",
				"/\[font=([^\[\<]+?)\]/i",
				"/\[align=(left|center|right)\]/i",
				"/\[p=(\d{1,2}|null), (\d{1,2}|null), (left|center|right)\]/i",
				"/\[float=left\]/i",
				"/\[float=right\]/i"

				), array(
				"<font color=\"\\1\">",
				"<font style=\"color:\\1\">",
				"<font style=\"background-color:\\1\">",
				"<font style=\"background-color:\\1\">",
				"<font size=\"\\1\">",
				"<font style=\"font-size:\\1\">",
				"<font face=\"\\1\">",
				"<p align=\"\\1\">",
				"<p style=\"line-height:\\1px;text-indent:\\2em;text-align:\\3\">",
				"<span style=\"float:left;margin-right:5px\">",
				"<span style=\"float:right;margin-left:5px\">"
				), $message));

			if($parsetype != 1) {
				if(strpos($msglower, '[/quote]') !== FALSE) {
					$message = preg_replace("/\s?\[quote\][\n\r]*(.+?)[\n\r]*\[\/quote\]\s?/is", '<div class="quote"><blockquote>\\1</blockquote></div>', $message);
				}
				if(strpos($msglower, '[/free]') !== FALSE) {
					$message = preg_replace("/\s*\[free\][\n\r]*(.+?)[\n\r]*\[\/free\]\s*/is", '<div class="quote"><blockquote>\\1</blockquote></div>', $message);
				}
			}
			if(strpos($msglower, '[/media]') !== FALSE) {
				$message = preg_replace("/\[media=([\w,]+)\]\s*([^\[\<\r\n]+?)\s*\[\/media\]/ies", $allowmediacode ? "\$this->parsemedia('\\1', '\\2')" : "\$this->bbcodeurl('\\2', '<a href=\"{url}\" target=\"_blank\">{url}</a>')", $message);
			}
			if(strpos($msglower, '[/audio]') !== FALSE) {
				$message = preg_replace("/\[audio(=1)*\]\s*([^\[\<\r\n]+?)\s*\[\/audio\]/ies", $allowmediacode ? "\$this->parseaudio('\\2', 400)" : "\$this->bbcodeurl('\\2', '<a href=\"{url}\" target=\"_blank\">{url}</a>')", $message);
			}
			if(strpos($msglower, '[/flash]') !== FALSE) {
				$message = preg_replace("/\[flash(=(\d+),(\d+))?\]\s*([^\[\<\r\n]+?)\s*\[\/flash\]/ies", $allowmediacode ? "\$this->parseflash('\\2', '\\3', '\\4');" : "\$this->bbcodeurl('\\4', '<a href=\"{url}\" target=\"_blank\">{url}</a>')", $message);
			}
		}

		if(!$bbcodeoff) {
			if($parsetype != 1 && strpos($msglower, '[swf]') !== FALSE) {
				$message = preg_replace("/\[swf\]\s*([^\[\<\r\n]+?)\s*\[\/swf\]/ies", "\$this->bbcodeurl('\\1', ' <img src=\"".WEB_URL."static/image/filetype/flash.gif\" align=\"absmiddle\" alt=\"\" /> <a href=\"{url}\" target=\"_blank\">Flash: {url}</a> ')", $message);
			}

			if(strpos($msglower, '[/img]') !== FALSE) {
				$message = preg_replace(array(
					"/\[img\]\s*([^\[\<\r\n]+?)\s*\[\/img\]/ies",
					"/\[img=(\d{1,4})[x|\,](\d{1,4})\]\s*([^\[\<\r\n]+?)\s*\[\/img\]/ies"
				), $allowimgcode ? array(
					"\$this->bbcodeurl('\\1', '<img src=\"{url}\" onload=\"thumbImg(this)\" alt=\"\" />')",
					"\$this->parseimg('\\1', '\\2', '\\3', ".intval($lazyload).")"
				) : array(
					"\$this->bbcodeurl('\\1', '<a href=\"{url}\" target=\"_blank\">{url}</a>')",
					"\$this->bbcodeurl('\\3', '<a href=\"{url}\" target=\"_blank\">{url}</a>')",
				), $message);
			}
		}

		for($i = 0; $i <= $this->pcodecount; $i++) {
			$message = str_replace("[\tDISCUZ_CODE_$i\t]", $this->codehtml[$i], $message);
		}

		unset($msglower);

		if($jammer) {
			$message = preg_replace("/\r\n|\n|\r/e", "\$this->jammer()", $message);
		}

		return $htmlon ? $message : nl2br(str_replace(array("\t", '   ', '  '), array('&nbsp; &nbsp; &nbsp; &nbsp; ', '&nbsp; &nbsp;', '&nbsp;&nbsp;'), $message));
	}

	function codedisp($code) {
		$this->pcodecount++;
		$code = shtmlspecialchars(str_replace('\\"', '"', preg_replace("/^[\n\r]*(.+?)[\n\r]*$/is", "\\1", $code)));
		$this->codehtml[$this->pcodecount] = '<div class="blockcode"><div id="code_'.random(3).'"><ol><li>'.str_replace("\n", "<li>", $code).'</ol></div>';
		$this->codecount++;
		return "[\tDISCUZ_CODE_".$this->pcodecount."\t]";
	}

	function parseurl($url, $text, $scheme) {
		if(!$url && preg_match("/((https?|ftp|gopher|news|telnet|rtsp|mms|callto|bctp|thunder|qqdl|synacast){1}:\/\/|www\.)[^\[\"']+/i", trim($text), $matches)) {
			$url = $matches[0];
			$length = 65;
			if(strlen($url) > $length) {
				$text = substr($url, 0, intval($length * 0.5)).' ... '.substr($url, - intval($length * 0.3));
			}
			return '<a href="'.(substr(strtolower($url), 0, 4) == 'www.' ? 'http://'.$url : $url).'" target="_blank">'.$text.'</a>';
		} else {
			$url = substr($url, 1);
			if(substr(strtolower($url), 0, 4) == 'www.') {
				$url = 'http://'.$url;
			}
			$url = !$scheme ? WEB_URL.$url : $url;
			return '<a href="'.$url.'" target="_blank">'.$text.'</a>';
		}
	}

	function parseflash($w, $h, $url) {
		$w = !$w ? 550 : $w;
		$h = !$h ? 400 : $h;
		preg_match("/((https?){1}:\/\/|www\.)[^\[\"']+/i", $url, $matches);
		$url = $matches[0];
		$randomid = 'swf_'.random(3);
		if(LIB('io.file')->ext($url) != 'flv') {
			return '<span id="'.$randomid.'"></span><script type="text/javascript" reload="1">$(\''.$randomid.'\').innerHTML=AC_FL_RunContent(\'width\', \''.$w.'\', \'height\', \''.$h.'\', \'allowNetworking\', \'internal\', \'allowScriptAccess\', \'never\', \'src\', \''.$url.'\', \'quality\', \'high\', \'bgcolor\', \'#ffffff\', \'wmode\', \'transparent\', \'allowfullscreen\', \'true\');</script>';
		} else {
			return '<span id="'.$randomid.'"></span><script type="text/javascript" reload="1">$(\''.$randomid.'\').innerHTML=AC_FL_RunContent(\'width\', \''.$w.'\', \'height\', \''.$h.'\', \'allowNetworking\', \'internal\', \'allowScriptAccess\', \'never\', \'src\', \''.WEB_URL.'static/image/common/flvplayer.swf\', \'flashvars\', \'file='.rawurlencode($url).'\', \'quality\', \'high\', \'wmode\', \'transparent\', \'allowfullscreen\', \'true\');</script>';
		}
	}

	function parseed2k($url) {
		list(,$type, $name, $size,) = explode('|', $url);
		$url = 'ed2k://'.$url.'/';
		$name = addslashes($name);
		if($type == 'file') {
			$ed2kid = 'ed2k_'.random(3);
			return '<a id="'.$ed2kid.'" href="'.$url.'" target="_blank"></a><script language="javascript">$(\''.$ed2kid.'\').innerHTML=htmlspecialchars(unescape(decodeURIComponent(\''.$name.'\')))+\' ('.sizecount($size).')\';</script>';
		} else {
			return '<a href="'.$url.'" target="_blank">'.$url.'</a>';
		}
	}

	function aidencode($aid, $type = 0, $tid = 0) {
		global $_G;
		$s = !$type ? $aid.'|'.substr(md5($aid.md5(WEB_AUTHKEY).TIMESTAMP.$this->uid), 0, 8).'|'.TIMESTAMP.'|'.$this->uid.'|'.$tid : $aid.'|'.md5($aid.md5(WEB_AUTHKEY).TIMESTAMP).'|'.TIMESTAMP;
		return rawurlencode(base64_encode($s));
	}

	function parseattachurl($aid, $ext) {
		$this->skipaidlist[] = $aid;
		return WEB_URL.'forum.php?mod=attachment&aid='.$this->aidencode($aid, $ext, $this->tid).($ext ? '&request=yes&_f=.'.$ext : '');
	}

	function parseemail($email, $text) {
		$text = str_replace('\"', '"', $text);
		if(!$email && preg_match("/\s*([a-z0-9\-_.+]+)@([a-z0-9\-_]+[.][a-z0-9\-_.]+)\s*/i", $text, $matches)) {
			$email = trim($matches[0]);
			return '<a href="mailto:'.$email.'">'.$email.'</a>';
		} else {
			return '<a href="mailto:'.substr($email, 1).'">'.$text.'</a>';
		}
	}

	function parsetable($width, $bgcolor, $message) {
		if(strpos($message, '[/tr]') === FALSE && strpos($message, '[/td]') === FALSE) {
			$rows = explode("\n", $message);
			$s = '<table cellspacing="0" class="t_table" '.
				($width == '' ? NULL : 'style="width:'.$width.'"').
				($bgcolor ? ' bgcolor="'.$bgcolor.'">' : '>');
			foreach($rows as $row) {
				$s .= '<tr><td>'.str_replace(array('\|', '|', '\n'), array('&#124;', '</td><td>', "\n"), $row).'</td></tr>';
			}
			$s .= '</table>';
			return $s;
		} else {
			if(!preg_match("/^\[tr(?:=([\(\)\s%,#\w]+))?\]\s*\[td([=\d,%]+)?\]/", $message) && !preg_match("/^<tr[^>]*?>\s*<td[^>]*?>/", $message)) {
				return str_replace('\\"', '"', preg_replace("/\[tr(?:=([\(\)\s%,#\w]+))?\]|\[td([=\d,%]+)?\]|\[\/td\]|\[\/tr\]/", '', $message));
			}
			if(substr($width, -1) == '%') {
				$width = substr($width, 0, -1) <= 98 ? intval($width).'%' : '98%';
			} else {
				$width = intval($width);
				$width = $width ? ($width <= 560 ? $width.'px' : '98%') : '';
			}
			return '<table cellspacing="0" class="t_table" '.
				($width == '' ? NULL : 'style="width:'.$width.'"').
				($bgcolor ? ' bgcolor="'.$bgcolor.'">' : '>').
				str_replace('\\"', '"', preg_replace(array(
						"/\[tr(?:=([\(\)\s%,#\w]+))?\]\s*\[td(?:=(\d{1,4}%?))?\]/ie",
						"/\[\/td\]\s*\[td(?:=(\d{1,4}%?))?\]/ie",
						"/\[tr(?:=([\(\)\s%,#\w]+))?\]\s*\[td(?:=(\d{1,2}),(\d{1,2})(?:,(\d{1,4}%?))?)?\]/ie",
						"/\[\/td\]\s*\[td(?:=(\d{1,2}),(\d{1,2})(?:,(\d{1,4}%?))?)?\]/ie",
						"/\[\/td\]\s*\[\/tr\]\s*/i"
					), array(
						"\$this->parsetrtd('\\1', '0', '0', '\\2')",
						"\$this->parsetrtd('td', '0', '0', '\\1')",
						"\$this->parsetrtd('\\1', '\\2', '\\3', '\\4')",
						"\$this->parsetrtd('td', '\\1', '\\2', '\\3')",
						'</td></tr>'
					), $message)
				).'</table>';
		}
	}

	function parsetrtd($bgcolor, $colspan, $rowspan, $width) {
		return ($bgcolor == 'td' ? '</td>' : '<tr'.($bgcolor ? ' style="background-color:'.$bgcolor.'"' : '').'>').'<td'.($colspan > 1 ? ' colspan="'.$colspan.'"' : '').($rowspan > 1 ? ' rowspan="'.$rowspan.'"' : '').($width ? ' width="'.$width.'"' : '').'>';
	}

	function parseaudio($url, $width = 400) {
		$ext = strtolower(substr(strrchr($url, '.'), 1, 5));
		switch($ext) {
			case 'mp3':
			case 'wma':
			case 'mid':
			case 'wav':
				return '<object classid="clsid:6BF52A52-394A-11d3-B153-00C04F79FAA6" width="'.$width.'" height="64"><param name="invokeURLs" value="0"><param name="autostart" value="0" /><param name="url" value="'.$url.'" /><embed src="'.$url.'" autostart="0" type="application/x-mplayer2" width="'.$width.'" height="64"></embed></object>';
			case 'ra':
			case 'rm':
			case 'ram':
				$mediaid = 'media_'.random(3);
				return '<object classid="clsid:CFCDAA03-8BE4-11CF-B84B-0020AFBBCCFA" width="'.$width.'" height="32"><param name="autostart" value="0" /><param name="src" value="'.$url.'" /><param name="controls" value="controlpanel" /><param name="console" value="'.$mediaid.'_" /><embed src="'.$url.'" autostart="0" type="audio/x-pn-realaudio-plugin" controls="ControlPanel" console="'.$mediaid.'_" width="'.$width.'" height="32"></embed></object>';
		}
	}

	function parsemedia($params, $url) {
		$params = explode(',', $params);
		$width = intval($params[1]) > 800 ? 800 : intval($params[1]);
		$height = intval($params[2]) > 600 ? 600 : intval($params[2]);
		$url = addslashes($url);
		if($flv = $this->parseflv($url, $width, $height)) {
			return $flv;
		}
		if(in_array(count($params), array(3, 4))) {
			$type = $params[0];
			$url = str_replace(array('<', '>'), '', str_replace('\\"', '\"', $url));
			switch($type) {
				case 'mp3':
				case 'wma':
				case 'ra':
				case 'ram':
				case 'wav':
				case 'mid':
					return $this->parseaudio($url, $width);
				case 'rm':
				case 'rmvb':
				case 'rtsp':
					$mediaid = 'media_'.random(3);
					return '<object classid="clsid:CFCDAA03-8BE4-11cf-B84B-0020AFBBCCFA" width="'.$width.'" height="'.$height.'"><param name="autostart" value="0" /><param name="src" value="'.$url.'" /><param name="controls" value="imagewindow" /><param name="console" value="'.$mediaid.'_" /><embed src="'.$url.'" autostart="0" type="audio/x-pn-realaudio-plugin" controls="imagewindow" console="'.$mediaid.'_" width="'.$width.'" height="'.$height.'"></embed></object><br /><object classid="clsid:CFCDAA03-8BE4-11CF-B84B-0020AFBBCCFA" width="'.$width.'" height="32"><param name="src" value="'.$url.'" /><param name="controls" value="controlpanel" /><param name="console" value="'.$mediaid.'_" /><embed src="'.$url.'" autostart="0" type="audio/x-pn-realaudio-plugin" controls="controlpanel" console="'.$mediaid.'_" width="'.$width.'" height="32"></embed></object>';
				case 'flv':
					$randomid = 'flv_'.random(3);
					return '<span id="'.$randomid.'"></span><script type="text/javascript" reload="1">$(\''.$randomid.'\').innerHTML=AC_FL_RunContent(\'width\', \''.$width.'\', \'height\', \''.$height.'\', \'allowNetworking\', \'internal\', \'allowScriptAccess\', \'never\', \'src\', \''.WEB_URL.'static/image/common/flvplayer.swf\', \'flashvars\', \'file='.rawurlencode($url).'\', \'quality\', \'high\', \'wmode\', \'transparent\', \'allowfullscreen\', \'true\');</script>';
				case 'swf':
					$randomid = 'swf_'.random(3);
					return '<span id="'.$randomid.'"></span><script type="text/javascript" reload="1">$(\''.$randomid.'\').innerHTML=AC_FL_RunContent(\'width\', \''.$width.'\', \'height\', \''.$height.'\', \'allowNetworking\', \'internal\', \'allowScriptAccess\', \'never\', \'src\', \''.$url.'\', \'quality\', \'high\', \'bgcolor\', \'#ffffff\', \'wmode\', \'transparent\', \'allowfullscreen\', \'true\');</script>';
				case 'asf':
				case 'asx':
				case 'wmv':
				case 'mms':
				case 'avi':
				case 'mpg':
				case 'mpeg':
					return '<object classid="clsid:6BF52A52-394A-11d3-B153-00C04F79FAA6" width="'.$width.'" height="'.$height.'"><param name="invokeURLs" value="0"><param name="autostart" value="0" /><param name="url" value="'.$url.'" /><embed src="'.$url.'" autostart="0" type="application/x-mplayer2" width="'.$width.'" height="'.$height.'"></embed></object>';
				case 'mov':
					return '<object classid="clsid:02BF25D5-8C17-4B23-BC80-D3488ABDDC6B" width="'.$width.'" height="'.$height.'"><param name="autostart" value="false" /><param name="src" value="'.$url.'" /><embed src="'.$url.'" autostart="false" type="video/quicktime" controller="true" width="'.$width.'" height="'.$height.'"></embed></object>';
				default:
					return '<a href="'.$url.'" target="_blank">'.$url.'</a>';
			}
		}
		return;
	}

	function bbcodeurl($url, $tags) {
		if(!preg_match("/<.+?>/s", $url)) {
			if(!in_array(strtolower(substr($url, 0, 6)), array('http:/', 'https:', 'ftp://', 'rtsp:/', 'mms://')) && !preg_match('/^static\//', $url) && !preg_match('/^data\//', $url)) {
				$url = 'http://'.$url;
			}
			return str_replace(array('submit', 'member.php?mod=logging'), array('', ''), str_replace('{url}', addslashes($url), $tags));
		} else {
			return '&nbsp;'.$url;
		}
	}

	function jammer() {
		$randomstr = '';
		for($i = 0; $i < mt_rand(5, 15); $i++) {
			$randomstr .= chr(mt_rand(32, 59)).' '.chr(mt_rand(63, 126));
		}
		return mt_rand(0, 1) ? '<font class="jammer">'.$randomstr.'</font>'."\r\n" :
			"\r\n".'<span style="display:none">'.$randomstr.'</span>';
	}

	function parseflv($url, $width = 0, $height = 0) {
		$lowerurl = strtolower($url);
		$flv = '';
		$imgurl = '';
		if($lowerurl != str_replace(array('player.youku.com/player.php/sid/','tudou.com/v/','player.ku6.com/refer/'), '', $lowerurl)) {
			$flv = $url;
		} elseif(strpos($lowerurl, 'v.youku.com/v_show/') !== FALSE) {
			if(preg_match("/http:\/\/v.youku.com\/v_show\/id_([^\/]+)(.html|)/i", $url, $matches)) {
				$flv = 'http://player.youku.com/player.php/sid/'.$matches[1].'/v.swf';
				if(!$width && !$height) {
					$api = 'http://v.youku.com/player/getPlayList/VideoIDS/'.$matches[1];
					$str = stripslashes(file_get_contents($api));
					if(!empty($str) && preg_match("/\"logo\":\"(.+?)\"/i", $str, $image)) {
						$url = substr($image[1], 0, strrpos($image[1], '/')+1);
						$filename = substr($image[1], strrpos($image[1], '/')+2);
						$imgurl = $url.'0'.$filename;
					}
				}
			}
		} elseif(strpos($lowerurl, 'tudou.com/programs/view/') !== FALSE) {
			if(preg_match("/http:\/\/(www.)?tudou.com\/programs\/view\/([^\/]+)/i", $url, $matches)) {
				$flv = 'http://www.tudou.com/v/'.$matches[2];
				if(!$width && !$height) {
					$str = file_get_contents($url);
					if(!empty($str) && preg_match("/<span class=\"s_pic\">(.+?)<\/span>/i", $str, $image)) {
						$imgurl = trim($image[1]);
					}
				}
			}
		} elseif(strpos($lowerurl, 'v.ku6.com/show/') !== FALSE) {
			if(preg_match("/http:\/\/v.ku6.com\/show\/([^\/]+).html/i", $url, $matches)) {
				$flv = 'http://player.ku6.com/refer/'.$matches[1].'/v.swf';
				if(!$width && !$height) {
					$api = 'http://vo.ku6.com/fetchVideo4Player/1/'.$matches[1].'.html';
					$str = file_get_contents($api);
					if(!empty($str) && preg_match("/\"picpath\":\"(.+?)\"/i", $str, $image)) {
						$imgurl = str_replace(array('\u003a', '\u002e'), array(':', '.'), $image[1]);
					}
				}
			}
		} elseif(strpos($lowerurl, 'v.ku6.com/special/show_') !== FALSE) {
			if(preg_match("/http:\/\/v.ku6.com\/special\/show_\d+\/([^\/]+).html/i", $url, $matches)) {
				$flv = 'http://player.ku6.com/refer/'.$matches[1].'/v.swf';
				if(!$width && !$height) {
					$api = 'http://vo.ku6.com/fetchVideo4Player/1/'.$matches[1].'.html';
					$str = file_get_contents($api);
					if(!empty($str) && preg_match("/\"picpath\":\"(.+?)\"/i", $str, $image)) {
						$imgurl = str_replace(array('\u003a', '\u002e'), array(':', '.'), $image[1]);
					}
				}
			}
		} elseif(strpos($lowerurl, 'www.youtube.com/watch?') !== FALSE) {
			if(preg_match("/http:\/\/www.youtube.com\/watch\?v=([^\/&]+)&?/i", $url, $matches)) {
				$flv = 'http://www.youtube.com/v/'.$matches[1].'&hl=zh_CN&fs=1';
				if(!$width && !$height) {
					$str = file_get_contents($url);
					if(!empty($str) && preg_match("/'VIDEO_HQ_THUMB':\s'(.+?)'/i", $str, $image)) {
						$url = substr($image[1], 0, strrpos($image[1], '/')+1);
						$filename = substr($image[1], strrpos($image[1], '/')+3);
						$imgurl = $url.$filename;
					}
				}
			}
		} elseif(strpos($lowerurl, 'tv.mofile.com/') !== FALSE) {
			if(preg_match("/http:\/\/tv.mofile.com\/([^\/]+)/i", $url, $matches)) {
				$flv = 'http://tv.mofile.com/cn/xplayer.swf?v='.$matches[1];
				if(!$width && !$height) {
					$str = file_get_contents($url);
					if(!empty($str) && preg_match("/thumbpath=\"(.+?)\";/i", $str, $image)) {
						$imgurl = trim($image[1]);
					}
				}
			}
		} elseif(strpos($lowerurl, 'v.mofile.com/show/') !== FALSE) {
			if(preg_match("/http:\/\/v.mofile.com\/show\/([^\/]+).shtml/i", $url, $matches)) {
				$flv = 'http://tv.mofile.com/cn/xplayer.swf?v='.$matches[1];
				if(!$width && !$height) {
					$str = file_get_contents($url);
					if(!empty($str) && preg_match("/thumbpath=\"(.+?)\";/i", $str, $image)) {
						$imgurl = trim($image[1]);
					}
				}
			}
		} elseif(strpos($lowerurl, 'you.video.sina.com.cn/b/') !== FALSE) {
			if(preg_match("/http:\/\/you.video.sina.com.cn\/b\/(\d+)-(\d+).html/i", $url, $matches)) {
				$flv = 'http://vhead.blog.sina.com.cn/player/outer_player.swf?vid='.$matches[1];
				if(!$width && !$height) {
					$api = 'http://interface.video.sina.com.cn/interface/common/getVideoImage.php?vid='.$matches[1];
					$str = file_get_contents($api);
					if(!empty($str)) {
						$imgurl = str_replace('imgurl=', '', trim($str));
					}
				}
			}
		} elseif(strpos($lowerurl, 'http://v.blog.sohu.com/u/') !== FALSE) {
			if(preg_match("/http:\/\/v.blog.sohu.com\/u\/[^\/]+\/(\d+)/i", $url, $matches)) {
				$flv = 'http://v.blog.sohu.com/fo/v4/'.$matches[1];
				if(!$width && !$height) {
					$api = 'http://v.blog.sohu.com/videinfo.jhtml?m=view&id='.$matches[1].'&outType=3';
					$str = file_get_contents($api);
					if(!empty($str) && preg_match("/\"cutCoverURL\":\"(.+?)\"/i", $str, $image)) {
						$imgurl = str_replace(array('\u003a', '\u002e'), array(':', '.'), $image[1]);
					}
				}
			}
		} elseif(strpos($lowerurl, 'http://www.ouou.com/fun_funview') !== FALSE) {
			$str = file_get_contents($url);
			if(!empty($str) && preg_match("/var\sflv\s=\s'(.+?)';/i", $str, $matches)) {
				$flv = WEB_URL.'static/image/common/flvplayer.swf?&autostart=true&file='.urlencode($matches[1]);
				if(!$width && !$height && preg_match("/var\simga=\s'(.+?)';/i", $str, $image)) {
					$imgurl = trim($image[1]);
				}
			}
		} elseif(strpos($lowerurl, 'http://www.56.com') !== FALSE) {

			if(preg_match("/http:\/\/www.56.com\/\S+\/play_album-aid-(\d+)_vid-(.+?).html/i", $url, $matches)) {
				$flv = 'http://player.56.com/v_'.$matches[2].'.swf';
				$matches[1] = $matches[2];
			} elseif(preg_match("/http:\/\/www.56.com\/\S+\/([^\/]+).html/i", $url, $matches)) {
				$flv = 'http://player.56.com/'.$matches[1].'.swf';
			}
			if(!$width && !$height && !empty($matches[1])) {
				$api = 'http://vxml.56.com/json/'.str_replace('v_', '', $matches[1]).'/?src=out';
				$str = file_get_contents($api);
				if(!empty($str) && preg_match("/\"img\":\"(.+?)\"/i", $str, $image)) {
					$imgurl = trim($image[1]);
				}
			}
		}
		if($flv) {
			if(!$width && !$height) {
				return array('flv' => $flv, 'imgurl' => $imgurl);
			} else {
				$width = addslashes($width);
				$height = addslashes($height);
				$flv = addslashes($flv);
				$randomid = 'flv_'.random(3);
				return '<span id="'.$randomid.'"></span><script type="text/javascript" reload="1">$(\''.$randomid.'\').innerHTML=AC_FL_RunContent(\'width\', \''.$width.'\', \'height\', \''.$height.'\', \'allowNetworking\', \'internal\', \'allowScriptAccess\', \'never\', \'src\', \''.$flv.'\', \'quality\', \'high\', \'bgcolor\', \'#ffffff\', \'wmode\', \'transparent\', \'allowfullscreen\', \'true\');</script>';
			}
		} else {
			return FALSE;
		}
	}

	function parseimg($width, $height, $src, $lazyload) {
		return $this->bbcodeurl($src, '<img'.($width > 0 ? ' width="'.$width.'"' : '').($height > 0 ? ' height="'.$height.'"' : '').' src="'.$src.'" border="0" alt="" />');
	}

	function parsesmiles(&$message) {
		static $enablesmiles;
		if($enablesmiles === null) {
			$enablesmiles = false;
			if(!empty($this->smilies) && is_array($this->smilies)) {
				foreach($this->smilies['replacearray'] AS $key => $smiley) {
					$this->smilies['replacearray'][$key] = '<img src="'.WEB_URL.'static/image/smiley/'.$this->smileytypes[$this->smilies['typearray'][$key]]['directory'].'/'.$smiley.'" smilieid="'.$key.'" border="0" alt="" />';
				}
				$enablesmiles = true;
			}
		}
		$enablesmiles && $message = preg_replace($this->smilies['searcharray'], $this->smilies['replacearray'], $message);
		return $message;
	}

	function get_smilies(){
		$query = DB()->select(array('table'=>'v_smilies','field'=>'*'),SQL_SELECT_QUERY);

		$this->smilies = array('searcharray' => array(), 'replacearray' => array(), 'typearray' => array());
		while($smiley = DB()->row($query)) {
			$this->smilies['searcharray'][$smiley['id']] = '/'.preg_quote(shtmlspecialchars($smiley['code']), '/').'/';
			$this->smilies['replacearray'][$smiley['id']] = $smiley['url'];
			$this->smilies['typearray'][$smiley['id']] = $smiley['typeid'];
		}
		$this->smileytypes=DB()->select(array('table'=>'v_imagetypes','field'=>'*'),SQL_SELECT_LIST,'typeid');
	}
}
