import bpy

# 씬의 모든 오브젝트를 순회합니다.
for obj in bpy.context.scene.objects:
    # 오브젝트가 메쉬 타입인 경우에만 처리합니다.
    if obj.type == 'MESH':
        original_mesh = obj.data
        bpy.context.view_layer.objects.active = obj
        obj.select_set(True)
        num_materials = len(original_mesh.materials)
        
        if num_materials > 1:
            # 각 재질에 대해 반복합니다.
            for mat_index in range(num_materials):
                # 선택된 모든 면을 해제합니다.
                bpy.ops.object.mode_set(mode='EDIT')
                bpy.ops.mesh.select_all(action='DESELECT')
                bpy.ops.object.mode_set(mode='OBJECT')
                
                # 현재 재질을 사용하는 면만 선택합니다.
                for face in obj.data.polygons:
                    if face.material_index == mat_index:
                        face.select = True
                
                # 선택된 면만으로 구성된 새로운 오브젝트를 생성합니다.
                bpy.ops.object.mode_set(mode='EDIT')
                bpy.ops.mesh.duplicate()
                bpy.ops.mesh.separate(type='SELECTED')
                bpy.ops.object.mode_set(mode='OBJECT')
                
                # 새로 생성된 오브젝트를 찾아 해당 재질만 남기고 나머지 재질을 제거합니다.
                new_obj = bpy.context.selected_objects[-1]  # 가장 최근에 생성된 오브젝트
                new_obj.data.materials.clear()  # 모든 재질을 제거
                new_obj.data.materials.append(original_mesh.materials[mat_index])  # 필요한 재질만 추가
                
            # 원본 오브젝트를 삭제합니다.
            bpy.data.objects.remove(obj, do_unlink=True)

print("작업 완료: 모든 메쉬 오브젝트들이 재질별로 분리되었으며, 각 오브젝트에는 해당 재질만 남아있습니다.")
