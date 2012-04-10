<?
if(!defined('IN_SITE'))
	exit('Access Denied');

import('lib.image.base');

/**
 * Enter description here ...
 * @author Administrator
 *
 */
class LibImageString extends LibImageBase{
	/**
	 +----------------------------------------------------------
	 * 根据给定的字符串生成图像
	 +----------------------------------------------------------
	 * @static
	 * @access public
	 +----------------------------------------------------------
	 * @param string $string  字符串
	 * @param string $size  图像大小 width,height 或者 array(width,height)
	 * @param string $font  字体信息 fontface,fontsize 或者 array(fontface,fontsize)
	 * @param string $type 图像格式 默认PNG
	 * @param integer $disturb 是否干扰 1 点干扰 2 线干扰 3 复合干扰 0 无干扰
	 * @param bool $border  是否加边框 array(color)
	 +----------------------------------------------------------
	 * @return string
	 +----------------------------------------------------------
	 */
	function string($string,$rgb=array(),$filename='',$type='png',$disturb=1,$border=true){
		if(is_string($size))		$size=	explode(',',$size);
		$width=	$size[0];
		$height=	$size[1];
		if(is_string($font))		$font=	explode(',',$font);
		$fontface=	$font[0];
		$fontsize=	$font[1];
		$length=	strlen($string);
		$width=($length*9+10)>$width?$length*9+10:$width;
		$height=	22;
		if($type!='gif' && function_exists('imagecreatetruecolor')){
			$im=@imagecreatetruecolor($width,$height);
		}else{
			$im=@imagecreate($width,$height);
		}
		if(empty($rgb)){
			$color=imagecolorallocate($im, 102, 104, 104);
		}else{
			$color=imagecolorallocate($im, $rgb[0], $rgb[1], $rgb[2]);
		}
		$backColor=imagecolorallocate($im, 255,255,255);	//背景色（随机）
		$borderColor=imagecolorallocate($im, 100, 100, 100);					//边框色
		$pointColor=imagecolorallocate($im,mt_rand(0,255),mt_rand(0,255),mt_rand(0,255));				 //点颜色

		@imagefilledrectangle($im, 0, 0, $width - 1, $height - 1, $backColor);
		@imagerectangle($im, 0, 0, $width-1, $height-1, $borderColor);
		@imagestring($im, 5, 5, 3, $string, $color);
		if(!empty($disturb)){
			// 添加干扰
			if($disturb=1 || $disturb=3){
				for($i=0;$i<25;$i++){
					imagesetpixel($im,mt_rand(0,$width),mt_rand(0,$height),$pointColor);
				}
			}elseif($disturb=2 || $disturb=3){
				for($i=0;$i<10;$i++){
					imagearc($im,mt_rand(-10,$width),mt_rand(-10,$height),mt_rand(30,300),mt_rand(20,200),55,44,$pointColor);
				}
			}
		}
		$this->output($im,$type,$filename);
	}
}
