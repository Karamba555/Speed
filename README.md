### Quickstart

Pull the latest updates for the feeds: `./scripts/feeds update -a`

Make the downloaded package/packages: `./scripts/feeds install -a`

For a clean installation the .config file must be restored after feeds update. Restore config file: `git restore .config`

Build the firmware image: `make -j$(nproc)`

Flashable images will be located in `Speedway/image` folder:

* `Speedway_Factory_*.bin` - factory image for tftp recovery
* `Speedway_Sysupgrade_*.bin` - sysupgrade image

### Image with read-only www folder
To build www folder with read-only permission the build process should be started with `www=ro` value, for example:
* `make -j$(nproc) V=sc www=ro`
To rebuild the project the `Speedway/build_dir/target-aarch64_cortex-a53_musl/root-mediatek/www` folder should be removed with root permission

### Changing firmware version
The build process uses the firmware version specified in the file **Version** in the root folder

### Additional Notes
Some build issues can be addresses by first running:
*make dirclean*
or
*make clean*

You must ensure that additional package(s) are installed on the linux system you are using to build.
First updates the list of available packages and upgrade packages:

    $ sudo apt-get update && sudo apt-get upgrade

Here is a list of required packages:

    gcc                 subversion        
    binutils            libz-dev
    bzip2               libc-dev 
    flex                rsync 
    python3             pip 
    perl                unzip 
    make                libncurses5-dev 
    grep                libncursesw5-dev 
    unzip               git 
    gawk                swig
    
    You can install these by typing:
    
    $ sudo apt-get install gcc binutils bzip2 flex python3 perl make grep unzip gawk subversion libz-dev libc-dev rsync pip unzip libncurses5-dev libncursesw5-dev git swig