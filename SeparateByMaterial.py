import bpy

# 씬의 모든 오브젝트를 순회합니다.
for obj in bpy.context.scene.objects:
    # 오브젝트가 메쉬 타입이고, 재질이 2개 이상인 경우에만 처리합니다.
    if obj.type == 'MESH' and len(obj.data.materials) > 1:
        # 원본 메쉬에 대한 참조를 저장합니다.
        original_mesh = obj.data
        
        # 원본 메쉬의 재질 개수를 확인합니다.
        num_materials = len(original_mesh.materials)
        
        # 각 재질에 대해 반복하여 메쉬를 복제하고, 해당 재질만을 보유하게 합니다.
        for mat_index in range(num_materials):
            # 원본 오브젝트를 복제합니다.
            new_obj = obj.copy()
            new_obj.data = original_mesh.copy()
            new_obj.animation_data_clear()
            bpy.context.collection.objects.link(new_obj)  # 씬에 새 오브젝트를 추가합니다.
            
            # 복제된 오브젝트의 재질을 정리합니다.
            # 재질 리스트를 반대로 순회하면서 현재 인덱스가 아닌 재질을 제거합니다.
            for i in range(num_materials - 1, -1, -1):
                if i != mat_index:
                    new_obj.data.materials.pop(index=i)
            
            # 복제된 오브젝트의 이름을 설정합니다.
            new_obj.name = f"{obj.name}_Mat{mat_index}"
        
        # 원본 오브젝트는 유지하거나 필요에 따라 삭제합니다.
        bpy.data.objects.remove(obj)

print("작업 완료: 모든 메쉬 오브젝트들에 대해 재질별로 분리하였습니다.")