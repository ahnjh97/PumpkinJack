import bpy

# 모든 오브젝트를 순회합니다.
for obj in bpy.data.objects:
    # 오브젝트가 메쉬 타입인 경우에만 진행합니다.
    if obj.type == 'MESH':
        # 해당 메쉬의 모든 머티리얼 슬롯을 순회합니다.
        for slot in obj.material_slots:
            # 슬롯에 머티리얼이 할당되어 있는 경우에만 진행합니다.
            if slot.material:
                # 현재 머티리얼의 이름을 가져옵니다.
                mat_name = slot.material.name
                # 머티리얼 이름에서 마지막 '.'의 위치를 찾습니다.
                last_dot_idx = mat_name.rfind('.')
                # 마지막 '.'이 존재하는 경우
                if last_dot_idx != -1:
                    # 새로운 머티리얼 이름을 결정합니다. (마지막 '.' 이전까지)
                    new_mat_name = mat_name[:last_dot_idx]
                    # 새로운 이름을 가진 머티리얼을 찾습니다.
                    new_material = bpy.data.materials.get(new_mat_name)
                    # 해당 이름을 가진 머티리얼이 존재하는 경우
                    if new_material:
                        # 현재 슬롯의 머티리얼을 새로운 머티리얼로 변경합니다.
                        slot.material = new_material
                    else:
                         new_mat = bpy.data.materials.new(name=new_mat_name)
                    

print("작업 완료: 머티리얼 이름 수정 및 할당 완료.")

