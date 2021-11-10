#include <zec/Common.hpp>
#include <iostream>
#include "./get_mac_if.hpp"
#include "./get_mac_rtnetlink.hpp"

using namespace std;

int main(int argc, char **  argv)
{
    cout << "List interfaces with mac: (by getifaddr)" << endl;
    auto ListIf = GetIfMac();
    for (auto & IF : ListIf) {
        cout << "Name:" << IF.Name;
        if (IF.HardwareAddress[0]) {
            cout << ", HWAddr:" << IF.HardwareAddress;
        }
        if (IF.InetAddress[0]) {
            cout << ", HWAddr:" << IF.InetAddress;
        }
        if (IF.Inet6Address[0]) {
            cout << ", HWAddr:" << IF.Inet6Address;
        }
        cout << endl;
    }
    cout << endl;

    cout << "List interfaces with mac: (by rtnetlink)" << endl;
    auto ListNetlink = GetNetlinkMac();
    for (auto & IF : ListNetlink) {
        cout << "Index:" << IF.Index << ", Name:" << IF.Name;
        if (IF.HardwareAddress[0]) {
            cout << ", HWAddr:" << IF.HardwareAddress;
        }
        cout << endl;
    }
    cout << endl;

    return 0;
}