#!/usr/bin/env python3
import re
import subprocess
import sys
import json
import os

def extract_md_offsets(md_path):
    offsets = {}
    current_struct = None
    
    with open(md_path, 'r') as f:
        for line in f:
            struct_match = re.match(r'## (Control Block|Participant Table|Frame Headers)', line)
            if struct_match:
                if 'Control Block' in line: current_struct = 'ControlBlock'
                elif 'Participant Table' in line: current_struct = 'ParticipantSlot'
                continue
                
            sub_match = re.match(r'### (Minimal|Standard|Traced) Header', line)
            if sub_match:
                current_struct = f"FrameHeader{sub_match.group(1)}"
                continue
                
            if current_struct and '|' in line and 'Offset' not in line and '---' not in line:
                parts = [p.strip() for p in line.split('|')]
                if len(parts) >= 5:
                    try:
                        offset = int(parts[1])
                        name = parts[4].replace('`', '')
                        if current_struct not in offsets:
                            offsets[current_struct] = {}
                        offsets[current_struct][name] = offset
                    except ValueError:
                        pass
    return offsets

def main():
    md_path = os.path.join(os.path.dirname(__file__), '..', 'docs', 'src', 'wire-format.md')
    md_offsets = extract_md_offsets(md_path)
    
    cpp_code = """
#include <iostream>
#include <cstddef>
#include <revenant/core/layout.hpp>

#define DUMP(struct_name, field) \\
    std::cout << "\\"" #struct_name "\\" : { \\"" #field "\\": " << offsetof(rvn::core::struct_name, field) << "}\\n";

int main() {
    DUMP(ControlBlock, magic);
    DUMP(ControlBlock, version);
    DUMP(ControlBlock, header_kind);
    DUMP(ControlBlock, capacity_log2);
    DUMP(ControlBlock, block_size_log2);
    DUMP(ControlBlock, pad0);
    DUMP(ControlBlock, published);
    DUMP(ControlBlock, epoch);
    DUMP(ControlBlock, pad1);
    DUMP(ControlBlock, reserved_pos);
    DUMP(ControlBlock, pad2);
    
    DUMP(ParticipantSlot, position);
    DUMP(ParticipantSlot, heartbeat_ns);
    DUMP(ParticipantSlot, pad0);
    DUMP(ParticipantSlot, slot_word);
    DUMP(ParticipantSlot, identity);
    DUMP(ParticipantSlot, lease_expiry_ns);
    DUMP(ParticipantSlot, registered_ns);
    DUMP(ParticipantSlot, name);
    DUMP(ParticipantSlot, pad1);
    
    DUMP(FrameHeaderMinimal, length);
    DUMP(FrameHeaderMinimal, type);
    DUMP(FrameHeaderMinimal, flags);
    DUMP(FrameHeaderMinimal, header_version);
    
    DUMP(FrameHeaderStandard, length);
    DUMP(FrameHeaderStandard, sequence);
    DUMP(FrameHeaderStandard, session);
    DUMP(FrameHeaderStandard, checksum);
    DUMP(FrameHeaderStandard, timestamp);
    
    DUMP(FrameHeaderTraced, length);
    DUMP(FrameHeaderTraced, producer_ts);
    DUMP(FrameHeaderTraced, trace_id);
    return 0;
}
    """
    
    with open('/tmp/dump_offsets.cpp', 'w') as f:
        f.write(cpp_code)
        
    include_dir = os.path.join(os.path.dirname(__file__), '..', 'include')
    subprocess.check_call(['g++', '-std=c++20', '-I', include_dir, '/tmp/dump_offsets.cpp', '-o', '/tmp/dump_offsets'])
    output = subprocess.check_output(['/tmp/dump_offsets'], text=True)
    
    cpp_offsets = {}
    for line in output.strip().split('\n'):
        obj = json.loads("{" + line + "}")
        for struct_name, fields in obj.items():
            if struct_name not in cpp_offsets:
                cpp_offsets[struct_name] = {}
            cpp_offsets[struct_name].update(fields)
            
    has_error = False
    for struct_name, fields in cpp_offsets.items():
        if struct_name not in md_offsets:
            print(f"Struct {struct_name} missing from docs")
            has_error = True
            continue
            
        for field, cpp_offset in fields.items():
            md_offset = md_offsets[struct_name].get(field)
            if md_offset != cpp_offset:
                print(f"Mismatch in {struct_name}::{field}: C++ says {cpp_offset}, docs say {md_offset}")
                has_error = True
                
    if has_error:
        sys.exit(1)
        
    print("All offsets match!")

if __name__ == '__main__':
    main()
