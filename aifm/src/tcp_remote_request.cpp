/*
*get:obj_id, ds_id, obj_id_len, device_id

//首先根据device_id确认数据在哪里，然后根据device_index选择ip绑定连接
//参考test_tcp文件 main()function

//连接建立：process——init（），把原来的farmemsize这个配置去掉
// TCPDevice(netaddr raddr, uint32_t num_connections, uint64_t far_mem_size);
// -> TCPRequestDevice(netaddr raddr, uint32_t num_connections);

// Request:
// |Opcode = KOpReadObject(1B) | ds_id(1B) | obj_id_len(1B) | obj_id |
// |Opcode = 操作符 | ds_id(1B) | obj_id_len(1B) | obj_id |
（其余三个参数通过指针获得，只需要把meta的这三个参数传过去就好
// Response:
// |data_len(2B)|data_buf(data_len B)| 
//所以需要组织一下拿回来的数据
//还有一个判断，判断数据对于caller程序来说是否是local的（先用device_index来判断吧）：
    （1）如果数据在caller的local_node，要用address把数据拿到，直接把数据取出来，通过tcp传回去；
    （2）如果数据本身在caller的远端，就通过object_id来获取；
*/