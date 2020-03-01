#! /bin/sh
# $Id: new_project.sh 118592 2008-01-30 17:37:54Z kazimird $
# Authors:  Denis Vakatov (vakatov@ncbi.nlm.nih.gov),
#           Aaron Ucko    (ucko@ncbi.nlm.nih.gov)

svn_revision=`echo '$Revision: 118592 $' | sed "s%\\$[R]evision: *\\([^$][^$]*\\) \\$.*%\\1%"`
def_builddir="$NCBI/c++/Debug/build"

repository_url='https://svn.ncbi.nlm.nih.gov/repos/toolkit'
tmp_app_checkout_dir='tmp_app_sample'

script=`basename $0`
timestamp=`date`
ws='[ 	]' # The brackets contain a space and a tab.

if test -x /usr/bin/nawk; then
   awk=nawk
else
   awk=awk
fi

#################################

Usage()
{
  cat <<EOF
USAGE:  $script <name> <type> [builddir]
SYNOPSIS:
   Create a model makefile "Makefile.<name>_<type>" to build
   a library or an application that uses pre-built NCBI C++ toolkit.
   Also include sample code when creating applications.
ARGUMENTS:
   <name>      -- name of the project (will be subst. to the makefile name)
   <type>      -- one of the following:
     lib             to build a simple library
     lib/asn         to build a library from an ASN.1 spec
     lib/dtd         to build a library from an XML DTD
     app[/basic]     to build a simple application
     app/alnmgr      to build an application using the alignment manager
     app/asn         to build a library from ASN.1 spec, and sample application
     app/blast       to build an application using BLAST
     app/cgi         to build a CGI or FastCGI application
     app/dbapi       to build a DBAPI application
     app/eutils      to build an eUtils client application
     app/gui         to build an FLTK application
     app/lds         to build an application using a local data store
     app/netschedule to build an NCBI GRID (NetSchedule) application
     app/objects     to build an application using ASN.1 objects
     app/objmgr      to build an application using the object manager
     app/soap/client to build a SOAP client
     app/soap/server to build a SOAP server
     app/unit_test   to build a Boost-based unit test application
   [builddir]  -- path to the pre-built NCBI C++ toolkit
                  (default = $def_builddir)

EOF

  test -z "$1"  ||  echo ERROR:  $1
  exit 1
}

#################################

CreateMakefile_Builddir()
{
  cat > $1 <<EOF
#
# Makefile:  Makefile.builddir
#
# This file was originally generated by shell script "$script" (r$svn_revision)
# ${timestamp}
#


###  PATH TO A PRE-BUILT C++ TOOLKIT  ###
builddir = $builddir
# builddir = \$(NCBI)/c++/Release/build
EOF
}

CreateMakefile_Lib()
{
  user_makefile="$1"
  proj_name="$2"
  proj_path="$3"
  proj_subtype="$4"
  lib_name="$5"

  case "$proj_type/$proj_subtype" in
   lib/?*)
     src="${lib_name}__ ${lib_name}___"
     cat > $user_makefile <<EOF
#
# Makefile:  $user_makefile
#
# This file was originally generated by shell script "$script" (r$svn_revision)
# ${timestamp}
#


###  PATH TO A PRE-BUILT C++ TOOLKIT  ###
include Makefile.builddir

`echo $proj_subtype | tr a-z A-Z`_PROJ = $lib_name

###  BUILD RULES -- DON'T EDIT OR MOVE THESE LINES !!!
srcdir = .
include \$(builddir)/Makefile.meta
MAKE_LIB = \$(MAKE) -f Makefile.\$\${i}_lib_ \$(MFLAGS)
requirements:;
EOF
     user_makefile=${user_makefile}_
     ;;
   app/asn)
     src="${lib_name}__ ${lib_name}___"
     ;;
   *)
     src=$lib_name
     ;;
  esac

  cat > "$user_makefile" <<EOF
#
# Makefile:  $user_makefile
#
# This file was originally generated by shell script "$script" (r$svn_revision)
# ${timestamp}
#


###  PATH TO A PRE-BUILT C++ TOOLKIT  ###
include Makefile.builddir


###  DEFAULT COMPILATION FLAGS -- DON'T EDIT OR MOVE THESE LINES !!!
include \$(builddir)/Makefile.mk
srcdir = .
BINCOPY = @:
BINTOUCH = @:
LOCAL_CPPFLAGS = -I. $extra_inc
LOCAL_LDFLAGS = -L.
ORIG_LDFLAGS = \$(LOCAL_LDFLAGS) \$(CONF_LDFLAGS)


#############################################################################
###  EDIT SETTINGS FOR THE DEFAULT (LIBRARY) TARGET HERE                  ### 

LIB = $lib_name
SRC = $src
# OBJ =

# CPPFLAGS = \$(ORIG_CPPFLAGS) \$(NCBI_C_INCLUDE)
# CFLAGS   = \$(ORIG_CFLAGS)
# CXXFLAGS = \$(ORIG_CXXFLAGS)
#
# LIB_OR_DLL = dll
#                                                                         ###
#############################################################################


###  LIBRARY BUILD RULES -- DON'T EDIT OR MOVE THESE 2 LINES !!!
include \$(builddir)/Makefile.is_dll_support
include \$(builddir)/Makefile.\$(LIB_OR_DLL)


###  PUT YOUR OWN ADDITIONAL TARGETS (MAKE COMMANDS/RULES) BELOW HERE
EOF
}



#################################

CreateMakefile_App()
{
  makefile_name="$1"
  proj_name="$2"
  proj_path="$3"
  proj_subtype="$4"
  app_name="$5"
  orig_app_name="$6"

  base=$src/app/sample/${proj_path}/Makefile.${orig_app_name}.app

  cat > "$makefile_name" <<EOF
#
# Makefile:  $makefile_name
#
# This file was originally generated by shell script "$script" (r$svn_revision)
# ${timestamp}
#


###  PATH TO A PRE-BUILT C++ TOOLKIT
include Makefile.builddir


###  DEFAULT COMPILATION FLAGS  -- DON'T EDIT OR MOVE THESE LINES !!!
include \$(builddir)/Makefile.mk
srcdir = .
BINCOPY = @:
LOCAL_CPPFLAGS = -I. $extra_inc
LOCAL_LDFLAGS = -L.
ORIG_LDFLAGS = \$(LOCAL_LDFLAGS) \$(CONF_LDFLAGS)
LDFLAGS = \$(ORIG_LDFLAGS)


#############################################################################
###  EDIT SETTINGS FOR THE DEFAULT (APPLICATION) TARGET HERE              ### 
APP = $app_name
SRC = `sed -ne "s/$old_proj_name/$proj_name/; s/^$ws*SRC$ws*=$ws*//p" $base`
# OBJ =

# PRE_LIBS = \$(NCBI_C_LIBPATH) .....

EOF

  $awk 'BEGIN { virtline = "" }
  /BEGIN COPIED SETTINGS/,/END COPIED SETTINGS/ {
     virtline = virtline $0 "\n"
     if (! /\\$/) {
       if (sub(/\$[({]FAST_LDFLAGS[)}]/, "$(LOCAL_LDFLAGS) &", virtline)) {
          printf "\n### LOCAL_LDFLAGS automatically added\n%s", virtline;
       } else if ( !/COPIED SETTINGS/ ) {
          printf "%s", virtline;
       }
       virtline = ""
     }      
  }' < "$base" >> "$makefile_name"

  cat >> "$makefile_name" <<EOF

# CFLAGS   = \$(ORIG_CFLAGS)
# CXXFLAGS = \$(ORIG_CXXFLAGS)
# LDFLAGS  = \$(ORIG_LDFLAGS)
#                                                                         ###
#############################################################################


###  APPLICATION BUILD RULES  -- DON'T EDIT OR MOVE THESE LINES !!!
include \$(builddir)/Makefile.app
MAKEFILE = `basename "$makefile_name"`


###  PUT YOUR OWN ADDITIONAL TARGETS (MAKE COMMANDS/RULES) HERE
EOF
}


CreateMakefile_Meta()
{
  makefile_name=$1
  proj_name=$2
  proj_path=$3
  old_proj_name=$4

  base=$src/app/sample/${proj_path}/Makefile.in

  cat > "$makefile_name" <<EOF
#
# Makefile:  $makefile_name
#
# This file was originally generated by shell script "$script" (r$svn_revision)
# ${timestamp}
#


###  PATH TO A PRE-BUILT C++ TOOLKIT
include Makefile.builddir

EOF
  sed -e 's,@builddir@,$(builddir),g; s,@srcdir@,.,g' \
      -e "s,$old_proj_name,$proj_name,g" < $base >> $makefile_name
  cat >> "$makefile_name" <<EOF

###  USE LOCAL TREE -- DON'T EDIT OR MOVE THESE TWO LINES !!!
MAKE_LIB = \$(MAKE) -f Makefile.\$\${i}_lib \$(MFLAGS)
MAKE_APP = \$(MAKE) -f Makefile.\$\${i}_app \$(MFLAGS)
EOF
}


Capitalize()
{
    echo $1 | awk 'BEGIN { FS = "[^A-Za-z0-9]+" }
        {
          for (i = 1;  i <= NF;  ++i) {
            printf("%s%s", toupper(substr($i, 1, 1)), tolower(substr($i, 2)));
          }
          print "";
        }'
}


#################################

case $# in
  3)  proj_name="$1" ; proj_type="$2" ; builddir="$3" ;;
  2)  proj_name="$1" ; proj_type="$2" ; builddir="$def_builddir" ;;
  0)  Usage "" ;;
  *)  Usage "Invalid number of arguments" ;;
esac

if test ! -d "$builddir"  ||  test ! -f "$builddir/../inc/ncbiconf_unix.h" -a ! -f "$builddir/../inc/ncbiconf.h" ; then
  Usage "Pre-built NCBI C++ toolkit is not found in:  \"$builddir\""
fi
case "$builddir" in
    /*) ;; # already absolute, no need to change
    *)  builddir=`(cd "${builddir}" && pwd)` ;;
esac

src=`sed -ne 's:^top_srcdir *= *\([^ ]*\):\1/src:p' < $builddir/Makefile.mk \
     | head -n 1`
test -n $src || src=${NCBI}/c++/src

case "${proj_type}" in
  app/*)
    proj_subdir=`echo ${proj_type} | sed -e 's@^app/@@'`
    proj_subtype=`echo ${proj_subdir} | tr / _`
    proj_type=app
    if test $proj_subtype = asn; then
      extra_inc="-I../../include -I../../include/$proj_name"
    fi
    ;;
  app)
    proj_subdir=basic
    proj_subtype=basic
    ;;
  lib/asn | lib/dtd)
    proj_subtype=`echo ${proj_type} | sed -e 's@^lib/@@'`
    proj_type=lib
    extra_inc="-I../../include -I../../include/$proj_name"
    ;;
  lib)
    proj_subtype=
    ;;
esac

if test "$proj_name" != `basename $proj_name` ; then
  Usage "Invalid project name:  \"$proj_name\""
fi

if test ! -d "$src/app/sample/$proj_subdir"; then
  svn co "$repository_url/trunk/c++/src/app/sample/$proj_subdir" \
    "$tmp_app_checkout_dir/app/sample/$proj_subdir"
  if test ! -d "$tmp_app_checkout_dir/app/sample/$proj_subdir"; then
    rm -rf "$tmp_app_checkout_dir"
    Usage "Unsupported application type ${proj_subdir}"
  fi
  src="`cd "$tmp_app_checkout_dir" && pwd`"
  cleanup='yes'
fi

makefile_name="Makefile.${proj_name}_${proj_type}"
touch tmp$$
if test ! -f ../$proj_name/tmp$$ ; then
   test -d $proj_name || mkdir $proj_name
fi
if test -n "$extra_inc" ; then
   if test -f ../../src/$proj_name/tmp$$; then
      mkdir -p ../../include/$proj_name
   elif test -f ../src/tmp$$; then
      mkdir -p ../include/$proj_name
   else
      mkdir -p $proj_name/include/$proj_name $proj_name/src/$proj_name
      cd $proj_name/src
      rm -f ../../tmp$$
   fi
fi
rm -f tmp$$
test -d $proj_name &&  makefile_name="$proj_name/$makefile_name"
makefile_name=`pwd`/$makefile_name
makefile_builddir=`dirname $makefile_name`/Makefile.builddir

if test -f $makefile_builddir ; then
  echo "\"$makefile_builddir\" already exists.  Do you want to override it?  [y/N]"
  read answer
  case "$answer" in
    [Yy]*) CreateMakefile_Builddir $makefile_builddir ;;
  esac
else
  CreateMakefile_Builddir $makefile_builddir
fi


if test -f $makefile_name ; then
  echo "\"$makefile_name\" already exists.  Do you want to override it?  [y/N]"
  read answer
  case "$answer" in
    [Yy]*) ;;
    *    ) exit 2 ;;
  esac
fi

old_proj_name=${proj_subtype}_sample

case "$proj_type" in
  lib )
    CreateMakefile_Lib $makefile_name $proj_name '' "$proj_subtype" $proj_name ;;
  app )
    CreateMakefile_App $makefile_name $proj_name $proj_subdir $proj_subtype $proj_name $old_proj_name ;;
  * )
    Usage "Invalid project type:  \"$proj_type\"" ;;
esac

echo "Created a model makefile \"$makefile_name\"."
def_makefile=`dirname $makefile_name`/Makefile

if test "$proj_type" != "app"; then
  test -f $def_makefile  ||  ln -s `basename $makefile_name` $def_makefile
  exit 0
fi


old_class_name=CSample`Capitalize ${proj_subtype}`Application
new_class_name=C`Capitalize ${proj_name}`Application

old_dir=${src}/app/sample/${proj_subdir}
if test -d "${proj_name}"; then
  new_dir=`pwd`/$proj_name
else
  new_dir=`pwd`
fi

CopySources()
{
  for input in $old_dir$1/*; do
    base=`basename $input | sed -e "s/${old_proj_name}/${proj_name}/g"`
    case $base in
      *~ | CVS | .svn | Makefile.${proj_name}.${proj_type})
        continue ;; # skip
    esac

    if grep -w "`basename $input`" $old_dir$1/.cvsignore >/dev/null 2>&1; then
        continue
    fi
  
    output=$new_dir$1/$base
    if test -d $input; then
      mkdir $output
      CopySources $1/$base
      continue
    elif test -f $output ; then
      echo "\"$output\" already exists.  Do you want to override it?  [y/n]"
      read answer
      case "$answer" in
        [Yy]*) ;;
        *    ) continue ;;
      esac
    fi
  
    case $input in
        */Makefile.*.app)
            this_proj=`basename $input | sed -e 's/Makefile\.\(.*\)\.app$/\1/'`
            this_proj_name=`echo $this_proj | sed -e "s/$old_proj_name/$proj_name/g"` 
            output=`echo $output | sed -e 's/\.app$/_app/'`
            case "$this_proj" in
                sample_*)
                    this_subtype=`echo $this_proj | sed -e 's/^sample_//'` ;;
                *)
                    this_subtype=$proj_subtype ;;
            esac
            CreateMakefile_App $output $proj_name $proj_subdir$1 $this_subtype \
                $this_proj_name $this_proj
            ;;
        */Makefile.*.lib)
            this_proj=`basename $input | sed -e 's/Makefile\.\(.*\)\.lib$/\1/'`
            this_proj_name=`echo $this_proj | sed -e "s/$old_proj_name/$proj_name/g"` 
            output=`echo $output | sed -e 's/\.lib$/_lib/'`
            case "$this_proj" in
                sample_*)
                    this_subtype=`echo $this_proj | sed -e 's/^sample_//'` ;;
                *)
                    this_subtype=$proj_subtype ;;
            esac
            CreateMakefile_Lib $output $proj_name $proj_subdir$1 $this_subtype \
                $this_proj_name
            ;;
        */Makefile.in)
            touch "$output"
            output=$new_dir$1/Makefile
            CreateMakefile_Meta $output $proj_name $proj_subdir$1 $old_proj_name
            test -r $new_dir$1/Makefile.builddir || \
                ln -s ../Makefile.builddir $new_dir$1/Makefile.builddir
            ;;
        *)
            sed -e "s/${old_proj_name}/${proj_name}/g" \
                -e "s/${old_class_name}/${new_class_name}/g" < $input > $output
            ;;
    esac
    test -x $input  &&  chmod +x $output
    
    case $output in
        */Makefile*) echo "Created a model makefile \"$output\"."    ;;
        *)           echo "Created a model source file \"$output\"." ;;
    esac
  done
}

CopySources ""
test -f $def_makefile  ||  ln -s `basename $makefile_name` $def_makefile
test -n "$cleanup" && rm -rf "$tmp_app_checkout_dir"