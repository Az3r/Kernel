<h1>Tham Khảo</h1>
<p><a href="http://derekmolloy.ie/writing-a-linux-kernel-module-part-2-a-character-device/">character device driver</a></p>
<p><a href="https://www.linuxtopia.org/online_books/Linux_Kernel_Module_Programming_Guide/x958.html">hook</a></p>
<p><a href="https://unix.stackexchange.com/questions/347245/i-want-to-know-which-process-open-the-kernel-device-driver">process's name, id</a></p>

<h1>CHARACTER DEVICE DRIVER</h1>
- vào thư mục ccd, gõ <b>make</b> để biên dịch và bắt đầu cài đặt

<h1>HOOK</h1>
- Vào thư mục hook, gõ lệnh <b>sudo cat /boot/System.map-$(uname -r) | grep sys_call_table  > sys_call_table_address.txt</b> để lấy địa chỉ của sys_call_table
- Thay thế địa chỉ của sys_call_table tại macro <b>SYS_CALL_TABLE_ADDRESS</b> trong file <b>hook.c</b>
- gõ <b>make</b> để biên dịch và bắt đầu cài đặt hook
