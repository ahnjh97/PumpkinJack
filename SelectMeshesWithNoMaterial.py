import bpy

# 씬의 모든 오브젝트를 순회하기 전에, 모든 오브젝트의 선택을 취소합니다.
bpy.ops.object.select_all(action='DESELECT')

# 선택된 메쉬의 수를 셀 변수입니다.
selected_mesh_count = 0

# 씬의 모든 오브젝트를 순회합니다.
for obj in bpy.context.scene.objects:
    # 오브젝트 타입이 메쉬인 경우에만 처리합니다.
    if obj.type == 'MESH':
        # 이미지 텍스쳐 노드가 연결되지 않았는지 확인하기 위한 플래그입니다.
        has_image_texture_node = False
        
        # 해당 메쉬 오브젝트의 모든 머티리얼을 순회합니다.
        if obj.data.materials:
            for mat in obj.data.materials:
                # 머티리얼이 노드를 사용하는 경우에만 처리합니다.
                if mat.use_nodes:
                    # 노드 트리를 가져옵니다.
                    nodes = mat.node_tree.nodes
                    # 노드 트리에서 'TEX_IMAGE' 타입의 노드(텍스쳐 노드)를 찾습니다.
                    for node in nodes:
                        if node.type == 'TEX_IMAGE':
                            has_image_texture_node = True
                            break # 이미지 텍스쳐 노드를 하나라도 찾으면 루프를 종료합니다.
                
                # 이미지 텍스쳐 노드가 하나라도 있는 경우, 더 이상 검사할 필요가 없습니다.
                if has_image_texture_node:
                    break
        
        # 이미지 텍스쳐 노드가 없는 경우, 해당 메쉬를 선택합니다.
        if not has_image_texture_node:
            obj.select_set(True)
            # 선택된 메쉬를 액티브 오브젝트로 설정합니다. (선택된 메쉬가 여러 개일 수 있으므로 마지막에 처리된 메쉬가 액티브 오브젝트가 됩니다.)
            bpy.context.view_layer.objects.active = obj
            # 선택된 메쉬의 수를 1 증가시킵니다.
            selected_mesh_count += 1

print(f"작업 완료: 이미지 텍스쳐 노드가 없는 메쉬 {selected_mesh_count}개를 선택하였습니다.")