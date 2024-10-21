#include "pch.hpp"

#include "command_queue.hpp"

#include "dx12_helpers.hpp"
#include "resource_util.hpp"

#include <queue>

using namespace Util;

CommandQueue::CommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2>& device, D3D12_COMMAND_LIST_TYPE type)
    : _fenceValue(0)
    , _commandListType(type)
    , _device(device)
{
    // Describe and create the command queue.
    // https://www.3dgep.com/learning-directx-12-1/#Command_Queue
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    ThrowIfFailed(_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&_commandQueue)));
    ThrowIfFailed(_device->CreateFence(_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence)));

    _fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(_fenceEvent && "Failed to create fence event handle.");
}

CommandQueue::~CommandQueue()
{
	WaitForFenceValue(_fenceValue);
	CloseHandle(_fenceEvent);
}

uint64_t CommandQueue::Signal()
{
    uint64_t fenceValue = ++_fenceValue;
    _commandQueue->Signal(_fence.Get(), fenceValue);
    return fenceValue;
}

bool CommandQueue::IsFenceComplete(uint64_t fenceValue)
{
    return _fence->GetCompletedValue() >= fenceValue;
}

void CommandQueue::WaitForFenceValue(uint64_t fenceValue)
{
    if (!IsFenceComplete(fenceValue))
    {
        _fence->SetEventOnCompletion(fenceValue, _fenceEvent);
        ::WaitForSingleObject(_fenceEvent, DWORD_MAX);
    }
}

void CommandQueue::Flush()
{
    WaitForFenceValue(Signal());
}

Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandQueue::CreateCommandAllocator()
{
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
    ThrowIfFailed(_device->CreateCommandAllocator(_commandListType, IID_PPV_ARGS(&commandAllocator)));

    return commandAllocator;
}

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CommandQueue::CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator)
{
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList;
    ThrowIfFailed(_device->CreateCommandList(0, _commandListType, allocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

    return commandList;
}

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CommandQueue::GetCommandList()
{
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList;

    if (!_commandAllocatorQueue.empty() && IsFenceComplete(_commandAllocatorQueue.front().fenceValue))
    {
        commandAllocator = _commandAllocatorQueue.front().commandAllocator;
        _commandAllocatorQueue.pop();

        ThrowIfFailed(commandAllocator->Reset());
    }
    else
    {
        commandAllocator = CreateCommandAllocator();
    }

    if (!_commandListQueue.empty())
    {
        commandList = _commandListQueue.front();
        _commandListQueue.pop();

        ThrowIfFailed(commandList->Reset(commandAllocator.Get(), nullptr));
    }
    else
    {
        commandList = CreateCommandList(commandAllocator);
    }

    // Associate the command allocator with the command list so that it can be
    // retrieved when the command list is executed.
    ThrowIfFailed(commandList->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), commandAllocator.Get()));

    return commandList;
}

// Execute a command list.
// Returns the fence value to wait for for this command list.
uint64_t CommandQueue::ExecuteCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    commandList->Close();

    ID3D12CommandAllocator* commandAllocator;
    UINT dataSize = sizeof(commandAllocator);
    ThrowIfFailed(commandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator));

    ID3D12CommandList* const ppCommandLists[] = {
        commandList.Get()
    };

    _commandQueue->ExecuteCommandLists(1, ppCommandLists);
    uint64_t fenceValue = Signal();

    _commandAllocatorQueue.emplace(CommandAllocatorEntry{ fenceValue, commandAllocator });
    _commandListQueue.push(commandList);

    // The ownership of the command allocator has been transferred to the ComPtr
    // in the command allocator queue. It is safe to release the reference 
    // in this temporary COM pointer here.
    commandAllocator->Release();

    return fenceValue;
}

Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue::GetCommandQueue() const
{
	return _commandQueue;
}