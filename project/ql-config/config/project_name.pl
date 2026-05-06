
use Cwd;

$CURDIR = getcwd();
$CURDIR =~ s/\//\\/g;
my $project_name = uc $ARGV[0]; #ËùÓÐÐ¡Ð´×ª´óÐ´
my $cust_ver =  uc $ARGV[1]; 
my $OEM_cfg = uc $ARGV[2];
my $QuecVerfilePath = "$CURDIR\\..\\..\\ql-config\\config\\$project_name\\include\\quectel_version.h";
my $CustVerfilePath = "$CURDIR\\..\\..\\ql-config\\config\\$project_name\\include\\ql_cust_version.h";
my $cp_prjname_file = "$CURDIR\\..\\..\\ql-config\\config\\$project_name\\include\\QuecPrjName.h";
my $app_prjname_file = "$CURDIR\\..\\..\\ql-application\\threadx\\common\\include\\QuecPrjName.h";
my $QUEC_CUR_PRJ_file = "$CURDIR\\..\\..\\ql-config\\config\\QuecCurPrj.txt";
my $QUEC_FLASH_Type_file = "$CURDIR\\..\\..\\ql-config\\config\\QuecFLashType.ini";
my $firmware_name_filepath = "$CURDIR\\..\\..\\ql-config\\quec-project\\scripts\\win32\\build_package.bat";
my $ram_size_file = "$CURDIR\\..\\..\\ql-kernel\\threadx\\config\\ram\\RAM_SIZE.ini";
my $flash_size_file = "$CURDIR\\..\\..\\ql-config\\config\\FLASH_SIZE.ini";


#Get Ver
print "\n================PROJECT_INFO================\n";
print "project_name:".$project_name."\n";
print "cust_ver:".$cust_ver."\n";
print "OEM_cfg:".$OEM_cfg."\n";
print "==============================================\n";

my $prjname_exit = 0;
my $PRJ_CUR = 0;
my $Ram_Size = "4M";
my $Flash_Size = "4M";
my $flash_type = "DEFAULT";

if($project_name =~ /EC200MCN_LA/)
{
    $prjname_exit = 1;
    $PRJ_CUR = $&;
    $Ram_Size = "8M";
    $Flash_Size = "8M";
}
elsif($project_name =~ /EC600MCN_LA/)
{
    $prjname_exit = 1;
    $PRJ_CUR = $&;
    $Ram_Size = "8M";
    $Flash_Size = "8M";
}
elsif($project_name =~ /EC600MCN_LC/)
{
    $prjname_exit = 1;
    $PRJ_CUR = $&;
    $Ram_Size = "4M";
     $Flash_Size = "4M";
}
elsif($project_name =~ /EC800MCN_LC/)
{
    $prjname_exit = 1;
    $PRJ_CUR = $&;
    $Ram_Size = "4M";
    $Flash_Size = "4M";
}
elsif($project_name =~ /EC800MCN_LA/)
{
    $prjname_exit = 1;
    $PRJ_CUR = $&;
    $Ram_Size = "8M";
    $Flash_Size = "8M";
}
elsif($project_name =~ /EC800MCN_GA/)
{
    $prjname_exit = 1;
    $PRJ_CUR = $&;
    $Ram_Size = "8M";
    $flash_type = "HX_GPS";
    $Flash_Size = "8M";
}
elsif($project_name =~ /EC800MCN_GB/)
{
    $prjname_exit = 1;
    $PRJ_CUR = $&;
    $Ram_Size = "8M";
    $flash_type = "HX_GPS";
    $Flash_Size = "8M";
}
elsif($project_name =~ /EC800MCN_GD/)
{
    $prjname_exit = 1;
    $PRJ_CUR = $&;
    $Ram_Size = "8M";
    $flash_type = "HX_GPS";
    $Flash_Size = "8M";
}
elsif($project_name =~ /EC800MCN_GC/)
{
    $prjname_exit = 1;
    $PRJ_CUR = $&;
    $Ram_Size = "4M";
    $flash_type = "HX_GPS";
    $Flash_Size = "4M";
}
elsif($project_name =~ /EC800MCN_GM/)
{
	$prjname_exit = 1;
    $PRJ_CUR = $&;
    $Ram_Size = "4M";
    $flash_type = "HX_GPS";
     $Flash_Size = "4M";
}
elsif($project_name =~ /EC200MCN_LC/)
{
	$prjname_exit = 1;
    $PRJ_CUR = $&;
    $Ram_Size = "4M";
     $Flash_Size = "4M";
}
elsif($project_name =~ /EC600MCN_LF/)
{
	$prjname_exit = 1;
    $PRJ_CUR = $&;
    $Ram_Size = "4M";
    $Flash_Size = "4M";
}
elsif($project_name =~ /EC800MCN_LF/)
{
	$prjname_exit = 1;
    $PRJ_CUR = $&;
    $Ram_Size = "4M";
    $Flash_Size = "4M";
}
elsif($project_name =~ /EC600MCN_CC/)
{
	$prjname_exit = 1;
    $PRJ_CUR = $&;
    $Ram_Size = "4M";
    $Flash_Size = "2M";
}
elsif($project_name =~ /EC200MCN_GB/)
{
	$prjname_exit = 1;
    $PRJ_CUR = $&;
    $Ram_Size = "8M";
    $Flash_Size = "8M";
    $flash_type = "ASR_GPS_BT";
}
else
{
	print "\n********************          ERROR          ***********************";
	print "\n********************          ERROR          ***********************";
	print "\n********************          ERROR          ***********************";
	print "\n********************          ERROR          ***********************";
	print "\n********************          ERROR          ***********************";
	print "\n********************          ERROR          ***********************";
	print "\n********************          ERROR          ***********************";
	print "\n     You must enter a project name [Not Support $project_name]    \n";
	print "\n********************          ERROR          ***********************";
	print "\n********************          ERROR          ***********************";
	print "\n********************          ERROR          ***********************";
	print "\n********************          ERROR          ***********************";
	print "\n********************          ERROR          ***********************";
	print "\n********************          ERROR          ***********************";
	print "\n********************          ERROR          ***********************\n";
	unlink("$QUEC_CUR_PRJ_file");
    $prjname_exit = 0;
    exit -1;
}

if($project_name =~ /BT/)
{
	$flash_type .= "_BT";
}

print "\n*****************PRJ_CUR=$PRJ_CUR****flash_type=$flash_type***********************\n";
my $PRJ_DEF = "#define __QUECTEL_PROJECT_".$PRJ_CUR."__";

my $Ver_OEM_Def = ""; 
if ($OEM_cfg ne "")
{
	$Ver_OEM_Def = "#define __QUEC_OEM_VER_".$OEM_cfg."__";
}

if($prjname_exit==1)
{
    open(PrjDefFile,">$cp_prjname_file");
    print PrjDefFile "/************************* This file is modify automaticly, please don't edit it! ***************************/\n\n";
    print PrjDefFile "#ifndef __PRJ_DEF_FILE__\n";
    print PrjDefFile "#define __PRJ_DEF_FILE__\n\n";
    print PrjDefFile "$PRJ_DEF\n\n";
		print PrjDefFile "$OemProjDef\n\n";
    print PrjDefFile "$Ver_OEM_Def\n\n";
    print PrjDefFile "#define MOB_SW_OEM \"$OEM_cfg\"\n\n";
    print PrjDefFile "#endif\n";

    print "\n==========================Build Project ".$PRJ_CUR."==============================\n\n";
    open(PrjName,">$QUEC_CUR_PRJ_file");
    print PrjName "$PRJ_CUR";
    close(PrjName);
    close(PrjDefFile);
    system("copy /Y \"$cp_prjname_file\" \"$app_prjname_file\"");
    
   	open(RAM_SIZE_FD,">$ram_size_file");
   	print RAM_SIZE_FD $Ram_Size;
    close(RAM_SIZE_FD);

    open(RAM_SIZE_FD,">$flash_size_file");
   	print RAM_SIZE_FD $Flash_Size;
    close(RAM_SIZE_FD);
    
    open(RAM_SIZE_FD,">$QUEC_FLASH_Type_file");
   	print RAM_SIZE_FD $flash_type;
    close(RAM_SIZE_FD);
}

if($cust_ver ne "")
{
    open(CustVerFile,">$CustVerfilePath");
    print CustVerFile "/************************* This file is cust_version by modifying in perl script ***************************/\n\n";
    print CustVerFile "#ifndef __QL_CUST_VERSION_H__\n";
    print CustVerFile "#define __QL_CUST_VERSION_H__\n\n";
    print CustVerFile "char mob_cust_rev[100] =\"$cust_ver\";\n";
    print CustVerFile "#endif\n";
    close(CustVerFile);
}
my $firmware_name = "";
open(CustVerFile,"<$CustVerfilePath");
while(<CustVerFile>)
{
    chomp;
    if($_=~ /mob_cust_rev/)
    {
        $_ =~ s/\s||;||\"//g;
        $_ =~ /\/\//;
        $_ =~ s/$'//;
        $_ =~ s/\/\///;
        $_ =~ s/charmob_cust_rev\[100\]\=//;
        if($_ ne "")
        { 
            print "\n==========cust_version :$_============\n" ;
            $firmware_name = $_;
        }
        else
        {
            print "cust_version not exist We will use the default version number";
            open (quecverFile ,"<$QuecVerfilePath");
            while(<quecverFile>)
            {
                chomp;
                if($_=~ /mob_sw_rev/)
                {
                    $_ =~ s/\s||;||\"//g;
                    $_ =~ s/charmob_sw_rev\[\]\=//;
                    $firmware_name = $_;
                    print "\n===============firmware_name:$firmware_name=============\n";
                    last;
                }
                
            }
            close(quecverFile);
        }
       last;
    }
}
close(CustVerFile);

if(-e $QuecVerfilePath)
{
 	open(FILE,"<",$QuecVerfilePath)||die"cannot open the file: $!\n";
 	@linelist=<FILE>;
   	my $t=0;
    foreach $eachline(@linelist){
        if($eachline=~/\Achar\smob_model_id\[\]\s=/)
        { 
            last;
        }
        $t=$t+1;  

    }
    close FILE;
    $linelist[$t]="char mob_model_id[] = \"$PRJ_CUR\"\;"."\r\n";
    open(FILE,">",$QuecVerfilePath)||die"cannot open the file: $!\n";
    print FILE @linelist;
    close FILE;
}
else
{
   	print "\n********************          ERROR          ***********************";
	print "\n                We didn't find any packaging scripts               \n";
	print "\n********************          ERROR          ***********************\n"; 
}

if(-e $firmware_name_filepath)
{
    @ARGV = "$firmware_name_filepath";
    local $^I='.bak';
    while (<>) {
            s/\Aset buildver=.*/set buildver=$firmware_name/;
            print;
    }
}
else
{
   	print "\n********************          ERROR          ***********************";
	print "\n                We didn't find any packaging scripts               \n";
	print "\n********************          ERROR          ***********************\n"; 
}

	

