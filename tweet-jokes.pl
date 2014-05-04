#!/usr/bin/env perl
open(JOKEF, "jokes.txt") || die "failed to open jokes.txt";

while(<JOKEF>){
	$jokes[$#jokes+1]=$_;
}

close(JOKEF);

$e=int(rand($#jokes+1));
$r=system("ttytter -keyf=coma_ararat -status=\"$jokes[$e]\"");
if($r!=0){
	# fallback to fortune
	$str=`fortune -a -n 140 -s`;
	`ttytter -keyf=coma_ararat -hold -status="$str"`;
}
