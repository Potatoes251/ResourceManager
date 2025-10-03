#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <iostream>
#include <fstream>	// open file
#include <sstream>	// string stream

class IResource
{
	public:
		virtual ~IResource() = default;
};




class ResourceManager
{
	public :
		ResourceManager() = default;
		~ResourceManager() = default;

		void	Delete(std::string id)
		{
			m_resourceMap.erase(id);
		}

		void	clearAll()
		{
			m_resourceMap.clear();	// Explicitly destroys all unique_ptr-held resources
		}

		template <typename T>
		T*		Create(std::string id)
		{
			if (Get<T>(id) != nullptr)
			{
				Delete(id);
			}

			std::unique_ptr<T> resource = std::make_unique<T>(id);
			if (resource)
			{
				T* rawPtr = resource.get();
				m_resourceMap[id] = std::move(resource);
				return rawPtr;
			}
			else
			{
				return nullptr;
			}
		}

		template <typename T>
		T*		Get(std::string id)
		{
			auto it = m_resourceMap.find(id);

			if (it != m_resourceMap.end())
			{
				return dynamic_cast<T*>(it->second.get());
			}
			else
			{
				return nullptr;
			}
		}

	private :
		std::unordered_map<std::string, std::unique_ptr<IResource>> m_resourceMap;
};