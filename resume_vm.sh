vmName=vm1
ifpause=$(virsh list | grep paused)

if [ -n "$ifpause" ]; then
	virsh resume $vmName
fi
