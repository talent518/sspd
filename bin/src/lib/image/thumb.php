<?
if (  ! defined('IN_SITE') )
	exit('Access Denied');

import('lib.image.base');

/**
 * Enter description here .
 * ..
 * 
 * @author Administrator
 *        
 */
class LibImageThumb extends LibImageBase {

	var $info;

	/**
	 * Enter description here .
	 * ..
	 * 
	 * @param unknown_type $image
	 * @return boolean
	 */
	function open ( $image ) {
		$this->info = $this->getInfo($image);
		if ( $this->info['type'] == 'bmp' )
			include_once ( './gd_bmp.class.php' );
			
			// 载入原图
		$createFun = 'ImageCreateFrom' . ( $this->info['type'] == 'jpg' ? 'jpeg' : $this->info['type'] );
		if ( function_exists($createFun) ) {
			$this->imsrc = $createFun($image);
			return true;
		} else
			return false;
	}

	/**
	 * Enter description here .
	 * ..
	 * 
	 * @param unknown_type $thumbfile
	 * @param unknown_type $maxWidth
	 * @param unknown_type $maxHeight
	 * @param unknown_type $center
	 * @param unknown_type $bgcolor
	 * @return unknown boolean
	 */
	function save ( $thumbfile, $maxWidth = 100, $maxHeight = 100, $center = true, $bgcolor = 0xFFFFFF ) {
		if ( $this->info !== false ) {
			$srcWidth = $this->info['width'];
			$srcHeight = $this->info['height'];
			$type = ( $this->info['type'] == 'jpeg' ? 'jpg' : $this->info['type'] );
			$scale = min($maxWidth / $srcWidth, $maxHeight / $srcHeight); // 计算缩放比例
			if ( $scale >= 1 ) { // 超过原图大小不再缩略
				$width = $srcWidth;
				$height = $srcHeight;
			} else { // 缩略图尺寸
				$width = ( int ) ( $srcWidth * $scale );
				$height = ( int ) ( $srcHeight * $scale );
			}
			
			// 创建缩略图
			if ( $type != 'gif' && function_exists('imagecreatetruecolor') )
				$tgimg = imagecreatetruecolor($center ? $maxWidth : $width, $center ? $maxHeight : $height);
			else
				$tgimg = imagecreate($center ? $maxWidth : $width, $center ? $maxHeight : $height);
			
			if ( $center ) {
				$bgcolor = is_string($bgcolor) ? ( int ) str_replace('#', '0x', $bgcolor) : $bgcolor;
				$bgcolor = imagecolorallocate($tgimg, $bgcolor >> 16, $bgcolor >> 8 & 255, $bgcolor & 255);
				imagefilledrectangle($tgimg, 0, 0, $center ? $maxWidth : $width, $center ? $maxHeight : $height, $bgcolor);
			}
			
			// 复制图片
			if ( function_exists("ImageCopyResampled") )
				imagecopyresampled($tgimg, $this->imsrc, $center ? ( int ) ( ( $maxWidth - $width ) / 2 ) : 0, $center ? ( int ) ( ( $maxHeight - $height ) / 2 ) : 0, 0, 0, $width, $height, $srcWidth, $srcHeight);
			else
				imagecopyresized($tgimg, $this->imsrc, $center ? ( int ) ( ( $maxWidth - $width ) / 2 ) : 0, $center ? ( int ) ( ( $maxHeight - $height ) / 2 ) : 0, 0, 0, $width, $height, $srcWidth, $srcHeight);
			if ( 'gif' == $type || 'png' == $type ) {
				imagealphablending($tgimg, false); // 取消默认的混色模式
				imagesavealpha($tgimg, true); // 设定保存完整的 alpha 通道信息
			}
			
			// 对jpeg图形设置隔行扫描
			if ( 'jpg' == $type || 'jpeg' == $type )
				imageinterlace($tgimg, true);
				
				// 生成图片
			$imageFun = 'image' . ( $type == 'jpg' ? 'jpeg' : $type );
			$return = $imageFun($tgimg, $thumbfile);
			imagedestroy($tgimg);
			return $return;
		}
		return false;
	}

	/**
	 * Enter description here .
	 * ..
	 */
	function close () {
		$this->info = '';
		imagedestroy($this->imsrc);
	}

}
