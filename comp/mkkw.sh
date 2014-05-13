sort keywords | awk '
  BEGIN { 
    KWH = "keyword.h";
    TBH = "table2.c";
    printf("/* This file is automatically generated from '\''keywords'\'' by mkkw.sh. */\n") > KWH;
    printf("/* This file is automatically generated from '\''keywords'\'' by mkkw.sh. */\n") > TBH;
    printf("%%{\n");
    printf("#include \"" KWH "\"\n");
    printf("%%}\n");
    printf("struct keyword { char *name;  int  val; };\n");
    printf("%%%%\n")
    val = 128;
  }
  $2 == "=" {
    printf("%s, T_%s\n", $1, $3);
    next
  }
  /^[^#]/ {
    u = ""
    for(i=1;i<=length($1);i++) { 
      t = substr($1,i,1)
      j = index( "abcdefghijklmnopqrstuvwxyz.", t)
      if( j == 0 ) { u = u t }
      else { u = u substr("ABCDEFGHIJKLMNOPQRSTUVWXYZ_", j, 1) } 
    }

    printf("#define\tT_%-18s0x%x\n", u, val) > KWH; 
    printf("%s, T_%s\n", $1, u); 
    printf("/* %02x: %-15s */ { %-18s, %4s, %6s, %s },\n", val, $1, $2, $3, $4, $5) > TBH;
    val++;
  }
  END {
    if( val > 256 ) {
	printf("Error: Too many keywords\n") > "/dev/tty"
    }
  }
' | gperf -D -p -t -G | \
sed 's/^static struct keyword wordlist/struct keyword wordlist/' > keyword.c