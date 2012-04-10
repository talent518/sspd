<?
if(!defined('IN_SITE'))
	exit('Access Denied');

import('lib.image.base');

/**
 * Enter description here ...
 * @author Administrator
 *
 */
class LibImageVerify extends LibImageBase{
	/**
	 * Enter description here ...
	 */
	function verify(){
		$cfg=M('setup')->get('verify');
		if($cfg['type']==4){
			$this->cnVerify($cfg['length']);
		}else{
			$this->enVerify($cfg['length'],$cfg['type']);
		}
	}

	/**
	 +----------------------------------------------------------
	 * 生成图像验证码
	 +----------------------------------------------------------
	 * @static
	 * @access public
	 +----------------------------------------------------------
	 * @param string $length  位数
	 * @param string $mode  类型
	 * @param string $type 图像格式
	 * @param string $width  宽度
	 * @param string $height  高度
	 +----------------------------------------------------------
	 * @return string
	 +----------------------------------------------------------
	 */
	function enVerify($length=4,$mode=1,$type='png',$width=180,$height=50,$fontface='simhei.ttf'){
		$randval=L('string')->rand($length,$mode);
		L('cookie')->set('verify',$randval);
		$width=($length*10+10)>$width?$length*10+10:$width;
		if($type!='gif' && function_exists('imagecreatetruecolor')){
			$im=@imagecreatetruecolor($width,$height);
		}else{
			$im=@imagecreate($width,$height);
		}
		$r=Array(225,255,255,223);
		$g=Array(225,236,237,255);
		$b=Array(225,236,166,125);
		$key=mt_rand(0,3);

		$backColor=imagecolorallocate($im, $r[$key],$g[$key],$b[$key]);	//背景色（随机）
		$borderColor=imagecolorallocate($im, 100, 100, 100);					//边框色
		$pointColor=imagecolorallocate($im,mt_rand(0,255),mt_rand(0,255),mt_rand(0,255));				 //点颜色

		@imagefilledrectangle($im, 0, 0, $width - 1, $height - 1, $backColor);
		@imagerectangle($im, 0, 0, $width-1, $height-1, $borderColor);
		$stringColor=imagecolorallocate($im,mt_rand(0,200),mt_rand(0,120),mt_rand(0,120));
		// 干扰
		for($i=0;$i<10;$i++){
			$fontcolor=imagecolorallocate($im,mt_rand(0,255),mt_rand(0,255),mt_rand(0,255));
			imagearc($im,mt_rand(-10,$width),mt_rand(-10,$height),mt_rand(30,300),mt_rand(20,200),55,44,$fontcolor);
		}
		for($i=0;$i<25;$i++){
			$fontcolor=imagecolorallocate($im,mt_rand(0,255),mt_rand(0,255),mt_rand(0,255));
			imagesetpixel($im,mt_rand(0,$width),mt_rand(0,$height),$pointColor);
		}
		for($i=0;$i<$length;$i++){
			//imagestring($im,5,$i*10+5,mt_rand(1,8),$randval{$i}, $stringColor);
			$fontcolor=imagecolorallocate($im,mt_rand(0,120),mt_rand(0,120),mt_rand(0,120)); //这样保证随机出来的颜色较深。
			//$codex=L('string')->cut($code,$i,1,CFG()->charset,'');
			imagettftext($im,mt_rand(30,40),mt_rand(-45,45),40*$i+20,40,$fontcolor,RES_FONT_DIR.$fontface,$randval{$i});
		}
//		@imagestring($im, 5, 5, 3, $randval, $stringColor);
		$this->output($im,$type);
	}

	/**
	 * 中文验证码
	 * @param unknown_type $length
	 * @param unknown_type $type
	 * @param unknown_type $width
	 * @param unknown_type $height
	 * @param unknown_type $fontface
	 */
	function cnVerify($length=4,$type='png',$width=180,$height=50,$fontface='simhei.ttf'){
		$code=L('string')->rand($length,STRING_RAND_CN,CFG()->charset);
		$width=($length*45)>$width?$length*45:$width;
		L('cookie')->set('verify',$code);
		$im=imagecreatetruecolor($width,$height);
		$borderColor=imagecolorallocate($im, 100, 100, 100);					//边框色
		$bkcolor=imagecolorallocate($im,250,250,250);
		imagefill($im,0,0,$bkcolor);
		@imagerectangle($im, 0, 0, $width-1, $height-1, $borderColor);
		// 干扰
		for($i=0;$i<15;$i++){
			$fontcolor=imagecolorallocate($im,mt_rand(0,255),mt_rand(0,255),mt_rand(0,255));
			imagearc($im,mt_rand(-10,$width),mt_rand(-10,$height),mt_rand(30,300),mt_rand(20,200),55,44,$fontcolor);
		}
		for($i=0;$i<255;$i++){
			$fontcolor=imagecolorallocate($im,mt_rand(0,255),mt_rand(0,255),mt_rand(0,255));
			imagesetpixel($im,mt_rand(0,$width),mt_rand(0,$height),$fontcolor);
		}
		if(!is_file(RES_FONT_DIR.$fontface)){
			$fontface='simhei.ttf';
		}
		for($i=0;$i<$length;$i++){
			$fontcolor=imagecolorallocate($im,mt_rand(0,120),mt_rand(0,120),mt_rand(0,120)); //这样保证随机出来的颜色较深。
			$codex=L('string')->cut($code,$i,1,CFG()->charset,'');
			imagettftext($im,mt_rand(20,30),mt_rand(-30,30),40*$i+20,40,$fontcolor,RES_FONT_DIR.$fontface,$codex);
		}
		$this->output($im,$type);
	}
}
