import random
from collections import OrderedDict
from typing import List, Tuple, Dict

class Frame: # represents a physical memory frame
    def __init__(self, page: int = -1, last_used: int = 0):
        self.page = page
        self.last_used = last_used
        self.valid = page != -1

class TLBEntry:
    def __init__(self, virtual_page: int, physical_frame: int, timestamp: int = 0):
        self.virtual_page = virtual_page
        self.physical_frame = physical_frame
        self.timestamp = timestamp

class vmsim:
    
    def __init__(self, physical_frames: int = 3, tlb_size: int = 4, page_size: int = 4096):
        self.physical_frames = physical_frames
        self.tlb_size = tlb_size
        self.page_size = page_size
        
        # physical memory (frames)
        self.frames = [Frame() for _ in range(physical_frames)]
        self.frame_count = 0
        
        # TLB
        self.tlb = []
        
        # page table (virtual page -> physical frame mapping)
        self.page_table = {}
        
        # statistics
        self.page_faults = 0
        self.tlb_hits = 0
        self.tlb_misses = 0
        self.memory_accesses = 0
        self.current_time = 0
    
    def vir_to_phys_address(self, virtual_addr: int) -> Tuple[int, bool, bool]:
        
        # translates virtual address to physical address
        # by returning (physical_address, tlb_hit, page_fault)
        
        self.memory_accesses += 1
        self.current_time += 1
        
        # extracts page number and offset
        virtual_page = virtual_addr // self.page_size
        offset = virtual_addr % self.page_size
        
        # checks TLB first
        tlb_hit = False
        physical_frame = None
        
        for entry in self.tlb:
            if entry.virtual_page == virtual_page:
                physical_frame = entry.physical_frame
                entry.timestamp = self.current_time
                tlb_hit = True
                self.tlb_hits += 1
                break
        
        if not tlb_hit:
            self.tlb_misses += 1
            
            # checks page table
            if virtual_page in self.page_table:
                physical_frame = self.page_table[virtual_page]
                page_fault = False
            else:
                # page fault
                physical_frame = self.handlePageFault(virtual_page)
                page_fault = True
                self.page_faults += 1
            
            # update TLB
            self.updateTLB(virtual_page, physical_frame)
        else:
            page_fault = False
        
        # calculates physical address
        physical_addr = physical_frame * self.page_size + offset
        return physical_addr, tlb_hit, page_fault
    
    def updateTLB(self, virtual_page: int, physical_frame: int):
        # update TLB with new translation
        # remove existing entry if another one exists
        self.tlb = [entry for entry in self.tlb if entry.virtual_page != virtual_page]
        
        # add new entry
        new_entry = TLBEntry(virtual_page, physical_frame, self.current_time)
        self.tlb.append(new_entry)
        
        # maintains TLB size
        if len(self.tlb) > self.tlb_size:
            self.tlb.sort(key=lambda x: x.timestamp)
            self.tlb = self.tlb[1:]  # Remove oldest
    
    def handlePageFault(self, virtual_page: int) -> int:
        #handles the page fault using custom algorithm and return physical frame number
        if self.frame_count < self.physical_frames:
            # free frame is available
            frame_index = self.frame_count
            self.frames[frame_index] = Frame(virtual_page, self.current_time)
            self.frame_count += 1
        else:
            # need to evict a page using custom algorithm
            frame_index = self.findEvictionCandidate()
            old_page = self.frames[frame_index].page
            
            # removes old mapping
            if old_page in self.page_table:
                del self.page_table[old_page]
            
            # removes from TLB
            self.tlb = [entry for entry in self.tlb if entry.virtual_page != old_page]
            
            # load new page
            self.frames[frame_index] = Frame(virtual_page, self.current_time)
        
        # update the page table
        self.page_table[virtual_page] = frame_index
        return frame_index
    
    def findEvictionCandidate(self) -> int:
        # custom algorithm that uses LRU fallback using FIFO structure.
        min_pages = min(2, self.frame_count)
        oldest_time = float('inf')
        candidate_index = 0
        
        for i in range(min_pages):
            if self.frames[i].last_used < oldest_time:
                oldest_time = self.frames[i].last_used
                candidate_index = i
        
        return candidate_index
    
    def get_stats(self) -> Dict:
        # returns the current statistics
        return {
            'page_faults': self.page_faults,
            'tlb_hits': self.tlb_hits,
            'tlb_misses': self.tlb_misses,
            'memory_accesses': self.memory_accesses,
            'page_fault_rate': self.page_faults / self.memory_accesses if self.memory_accesses > 0 else 0,
            'tlb_hit_rate': self.tlb_hits / self.memory_accesses if self.memory_accesses > 0 else 0
        }

class pagealgorithm:
    # simulator for each page replacement algorithm
    
    @staticmethod
    def fifo(pages: List[int], capacity: int) -> int:
        # FIFO page replacement algorithm
        frames = []
        page_faults = 0
        
        for page in pages:
            if page not in frames:
                page_faults += 1
                if len(frames) < capacity:
                    frames.append(page)
                else:
                    frames.pop(0)  # Remove first (oldest)
                    frames.append(page)
        
        return page_faults
    
    @staticmethod
    def lru(pages: List[int], capacity: int) -> int:
        # LRU page replacement algorithm
        frames = OrderedDict()
        page_faults = 0
        
        for page in pages:
            if page in frames:
                # move to end (most recently used)
                frames.move_to_end(page)
            else:
                page_faults += 1
                if len(frames) < capacity:
                    frames[page] = True
                else:
                    # remove least recently used (first item)
                    frames.popitem(last=False)
                    frames[page] = True
        
        return page_faults
    
    @staticmethod
    def custom_algorithm(pages: List[int], capacity: int) -> int:
        # Custom algorithm: FIFO implementation with LRU fallback for first 2
        frames = []
        last_used = {}
        page_faults = 0
        current_time = 0
        
        for page in pages:
            current_time += 1
            
            if page in frames:
                # Update last used time
                last_used[page] = current_time
            else:
                # Page fault
                page_faults += 1
                
                if len(frames) < capacity:
                    frames.append(page)
                    last_used[page] = current_time
                else:
                    # Find eviction candidate (LRU among first 2)
                    min_pages = min(2, len(frames))
                    oldest_time = float('inf')
                    evict_index = 0
                    
                    for i in range(min_pages):
                        frame_page = frames[i]
                        if last_used[frame_page] < oldest_time:
                            oldest_time = last_used[frame_page]
                            evict_index = i
                    
                    # Remove evicted page
                    evicted_page = frames.pop(evict_index)
                    del last_used[evicted_page]
                    
                    # Add new page
                    frames.append(page)
                    last_used[page] = current_time
        
        return page_faults

def main():
    
    # 3 reference strings used for the test case
    test_cases = [
        {
            'name': 'Test Case 1',
            'reference_string': [1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5],
            'frames': 3,
            'tlb_size': 4
        },
        {
            'name': 'Test Case 2', 
            'reference_string': [7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2],
            'frames': 3,
            'tlb_size': 2
        },
        {
            'name': 'Test Case 3',
            'reference_string': [1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 6, 7, 8],
            'frames': 3,
            'tlb_size': 6
        }
    ]
    
    for test_case in test_cases:
        print(f"\n{test_case['name']}:")
        print(f"Reference String: {test_case['reference_string']}")
        print(f"Physical Frames: {test_case['frames']}")
        print(f"TLB Size: {test_case['tlb_size']}")
        print("=" * 50)
        
        # Page Replacement Algorithms
        reference_string = test_case['reference_string']
        frames = test_case['frames']
        
        fifo_faults = pagealgorithm.fifo(reference_string, frames)
        lru_faults = pagealgorithm.lru(reference_string, frames)
        custom_faults = pagealgorithm.custom_algorithm(reference_string, frames)
        
        print("Page Replacement Results:")
        print(f"  FIFO Page Faults: {fifo_faults}")
        print(f"  LRU Page Faults: {lru_faults}")
        print(f"  Custom Page Faults: {custom_faults}")
        
        # address Translation with TLB
        vm = vmsim(
            physical_frames=frames,
            tlb_size=test_case['tlb_size'],
            page_size=1024
        )
        
        # converts page numbers to virtual addresses
        virtual_addresses = [page * 1024 for page in reference_string]
        
        # process all addresses
        for vaddr in virtual_addresses:
            vm.vir_to_phys_address(vaddr)
        
        # get and display statistics
        stats = vm.get_stats()
        print("\nAddress Translation with TLB Results:")
        print(f"  Page Faults: {stats['page_faults']}")
        print(f"  TLB Hits: {stats['tlb_hits']}")
        print(f"  TLB Misses: {stats['tlb_misses']}")
        print(f"  Page Fault Rate: {stats['page_fault_rate']:.2%}")
        print(f"  TLB Hit Rate: {stats['tlb_hit_rate']:.2%}")

if __name__ == "__main__":
    main()
