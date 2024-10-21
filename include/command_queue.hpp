#pragma once

class CommandQueue
{
public:
	CommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2>& device, D3D12_COMMAND_LIST_TYPE type);
	~CommandQueue();

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> GetCommandList();
	uint64_t ExecuteCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

	uint64_t Signal();
	bool IsFenceComplete(uint64_t fenceValue);
	void WaitForFenceValue(uint64_t fenceValue);
	void Flush();

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() const;
private:
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator();
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator);

	// Keep track of command allocators that are "in-flight"
	struct CommandAllocatorEntry
	{
		uint64_t fenceValue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	};

	using CommandAllocatorQueue = std::queue<CommandAllocatorEntry>;
	using CommandListQueue = std::queue< Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> >;

	D3D12_COMMAND_LIST_TYPE							_commandListType;
	Microsoft::WRL::ComPtr<ID3D12Device2>			_device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>		_commandQueue;
	Microsoft::WRL::ComPtr<ID3D12Fence>				_fence;
	HANDLE											_fenceEvent;
	uint64_t										_fenceValue;

	CommandAllocatorQueue							_commandAllocatorQueue;
	CommandListQueue								_commandListQueue;
};