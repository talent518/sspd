<?php
if (  ! defined('IN_SERVER') )
	exit('Access Denied');

class ModMail {

	var $error;

	function send ( $email, $subject, $body, $attachments = array() ) {
		return $this->sends(array(
			$email
		), $subject, $body, $attachments);
	}

	function sends ( $emails, $subject, $body, $attachments = array() ) {
		$mail = LIB('net.smtp');
		$mail->CharSet = 'utf-8';
		$mail->Encoding = 'base64';
		$mail->Mailer = MAIL_TYPE; // ʹ��SMTP
		$mail->SMTPSecure = MAIL_SECURE;
		$mail->Host = MAIL_HOST;
		$mail->Port = MAIL_PORT;
		$mail->SMTPAuth = MAIL_AUTH;
		$mail->Username = MAIL_USERNAME;
		$mail->Password = MAIL_PASSWORD;
		$mail->From = MAIL_FROM;
		$mail->FromName = MAIL_FROMNAME;
		foreach ( $emails as $email ) {
			if ( is_array($email) )
				extract($email);
			else
				$name = '';
			$mail->AddAddress($email, $name); // �ռ���email������
		}
		if ( MAIL_REPLY )
			$mail->AddReplyTo(MAIL_REPLY, MAIL_REPLYNAME);
		$mail->WordWrap = 50; // �趨 word wrap
		foreach ( $attachments as $attachment )
			$mail->AddAttachment($attachment); // ����
		$mail->IsHTML(true); // ��HTML����
		$mail->Subject = $subject;
		$mail->Body = $body; // HTML Body
		$mail->AltBody = "This is the body when user views in plain text format"; // ������ʱ��Body
		if (  ! $mail->Send() ) {
			$this->error = $mail->ErrorInfo;
			return false;
		} else {
			$this->error = '';
			return true;
		}
	}

}
