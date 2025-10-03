#include "resourceManager.h"


//template<typename T>
//T*		ResourceManager::Create(std::string id)
//{
//	if (Get<T>(id) == nullptr)
//	{
//		Delete(id);
//	}
//
//	std::unique_ptr<T> resource = std::make_unique<T>(id);
//	if (resource)
//	{
//		m_resourceMap[id] = resource;
//		return m_resourceMap[id];
//	}
//	else
//	{
//		return nullptr;
//	}
//}

//template<typename T>
//T*		ResourceManager::Get(std::string id)
//{
//	auto it = m_resourceMap.find(id);
//
//	if (it != m_resourceMap.end())
//	{
//		return it->second;
//	}
//	else
//	{
//		return nullptr;
//	}
//}


//void	ResourceManager::Delete(std::string id)
//{
//	m_resourceMap.erase(id);
//}
