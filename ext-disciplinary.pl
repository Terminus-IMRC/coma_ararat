use utf8;

$handle=sub{
	$ref=shift;

	# return if the user is protected
	return 0 if ($ref->{'user'}->{'protected'} eq 'true');
	# return if the tweet is RTed one
	return 0 if ($ref->{'retweeted_status'}->{'id_str'});

	# get screen_name of the tweet
	$sn=&descape($ref->{'user'}->{'screen_name'});
	$sn=~s#/#-#g;

	# return if the tweet is by me
	return 0 if ("${sn}" eq 'coma_ararat');

	$dtext=&descape($ref->{'text'});
	# return if the tweet does not contain jokes
	return 0 if (($dtext !~ /COMA/i) && ($dtext !~ /C○MA/i) && ($dtext !~ /独楽/i) && ($dtext !~ /まれいたそ/i));

	# disabled due to status duplicate issue
	## do silent reply
	#$str= "\@$sn";
	#&updatest($str, 1, $ref->{'id_str'});

	# disabled because this cannot treat tweets that have over 140-2+ chars
	## do quated RT
	#$str="\"\@$sn: $dtext\"";
	#&updatest($str, 1, $ref->{'id_str'});

	# do normal RT
	$str="$dtext";
	&updatest($str, 1, 0, undef, $ref->{'id_str'});

	# send DM to @n_IMRC with its tweeter name and its tweet ID
	unless("${sn}" eq 'n_IMRC'){
		$str="D n_IMRC disp: \@$sn: http://twitter.com/$sn/status/$ref->{'id_str'}";
		&updatest($str);
	}

	&defaulthandle($ref);
	return 1;
};

$conclude = sub {
	&defaultconclude;
};
