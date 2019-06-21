INSERT IGNORE INTO ssp_user (uid,gid,username,password,email)(SELECT UID,3,username,password,email FROM uc_members);
INSERT IGNORE INTO ssp_user_setting (uid)(SELECT uid FROM uc_members);
UPDATE ssp_user_setting SET expiry=UNIX_TIMESTAMP()+86400*1000,expiry_dateline=UNIX_TIMESTAMP(),gold=1,gold_dateline=UNIX_TIMESTAMP();
UPDATE ssp_user SET gid=3 WHERE gid=4;
UPDATE ssp_user a,uc_members b set a.username=b.username where a.uid=b.uid;
