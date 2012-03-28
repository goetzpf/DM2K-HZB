eval 'exec perl -S $0 ${1+"$@"}'  # -*- Mode: perl -*-
    if $running_under_some_shell; # makeBaseApp 

use strict;

my $current_enum = 0;
my $topic = "";
my $typename = "";

my $svt = "#ifndef ALLOCATE_STORAGE\n extern char *stringValueTable[NUMBER_STRING_VALUES];\n extern XmString xmStringValueTable[NUMBER_STRING_VALUES];\n#else\n char *stringValueTable[NUMBER_STRING_VALUES] = {";

while (<>) {
  chomp($_);

  if (/^\W+([0-9_A-Z]*)\W+(.*)/) {
    print "   $1 = $current_enum,\n";
    $current_enum++;
    $svt .= "\"" . $2 . ",";
  } elsif (/^([_a-zA-Z]*)\W+([_A-Z]*)/) {
    if ($current_enum != 0) {
      print "   LAST_$topic = ", $current_enum - 1, "\n";
      print "} $typename;\n";
      print "#define NUM_${topic}S (LAST_$topic - FIRST_$topic + 1)\n\n";
    }
    $typename = $1;
    $topic = $2;
    $svt .= "\n   ";
    print "typedef enum {\n   FIRST_$topic = $current_enum,\n"
  }
}

$svt .= "\n};\n#endif\n";

if ($current_enum != 0) {
  print "   LAST_$topic = ", $current_enum - 1, "\n";
  print "} $typename;\n";
  print "#define NUM_${topic}S (LAST_$topic - FIRST_$topic + 1)\n\n";
  print "#define NUMBER_STRING_VALUES ", $current_enum, "\n\n";
  print $svt;
}
