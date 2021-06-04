# ShopStub
ShopStub is a channel intended to properly install Homebrew applications directly to a user's SD card or USB drive.
It it intended to be installed as a WAD.

It stems from several needs:
 - The Wii Shop Channel does not allow direct file access on the SD card, leaving a requirement to install a bootstrap program to NAND.
 - However, we do not want to burden the user with other channels in order to use the Open Shop Channel. Such a bootstrap program must be tailored.
 - We need to have a unified codebase for all types of bootstrap programs, allowing us to build once and update universally.
 - We want to provide forwarders for Homebrew applications installed, leaving a need for a banner.

A typical usage flow for using this channel might be as follows:
1. The user, via the Open Shop Channel, installs this Homebrew title.
2. The WAD this bootstrapper has been packaged with contains:
  - a modified ticket
  - a banner suitable for a forwarder
  - a forwarder application - TODO: determine
  - the contents of the Homebrew application as a content
  and is now available on the user's NAND to launch.
3. This bootstrapper extracts the contents of said application to SD/USB, depending on what is available.
4. The program replaces itself with the above forwarder application, relieving the user's NAND of the size of the application.

TODO: Modify to what is fully implemented eventually.
