defmodule JlcHelper.JLCTest do
  use JlcHelper.DataCase

  alias JlcHelper.JLC

  describe "componens" do
    alias JlcHelper.JLC.Component

    import JlcHelper.JLCFixtures

    @invalid_attrs %{category1: nil, category2: nil, datasheet: nil, description: nil, joins: nil, library_type: nil, manufacturer: nil, mpn: nil, package: nil, part: nil, price: nil, stock: nil}

    test "list_componens/0 returns all componens" do
      component = component_fixture()
      assert JLC.list_componens() == [component]
    end

    test "get_component!/1 returns the component with given id" do
      component = component_fixture()
      assert JLC.get_component!(component.id) == component
    end

    test "create_component/1 with valid data creates a component" do
      valid_attrs = %{category1: "some category1", category2: "some category2", datasheet: "some datasheet", description: "some description", joins: 42, library_type: "some library_type", manufacturer: "some manufacturer", mpn: "some mpn", package: "some package", part: "some part", price: 120.5, stock: 42}

      assert {:ok, %Component{} = component} = JLC.create_component(valid_attrs)
      assert component.category1 == "some category1"
      assert component.category2 == "some category2"
      assert component.datasheet == "some datasheet"
      assert component.description == "some description"
      assert component.joins == 42
      assert component.library_type == "some library_type"
      assert component.manufacturer == "some manufacturer"
      assert component.mpn == "some mpn"
      assert component.package == "some package"
      assert component.part == "some part"
      assert component.price == 120.5
      assert component.stock == 42
    end

    test "create_component/1 with invalid data returns error changeset" do
      assert {:error, %Ecto.Changeset{}} = JLC.create_component(@invalid_attrs)
    end

    test "update_component/2 with valid data updates the component" do
      component = component_fixture()
      update_attrs = %{category1: "some updated category1", category2: "some updated category2", datasheet: "some updated datasheet", description: "some updated description", joins: 43, library_type: "some updated library_type", manufacturer: "some updated manufacturer", mpn: "some updated mpn", package: "some updated package", part: "some updated part", price: 456.7, stock: 43}

      assert {:ok, %Component{} = component} = JLC.update_component(component, update_attrs)
      assert component.category1 == "some updated category1"
      assert component.category2 == "some updated category2"
      assert component.datasheet == "some updated datasheet"
      assert component.description == "some updated description"
      assert component.joins == 43
      assert component.library_type == "some updated library_type"
      assert component.manufacturer == "some updated manufacturer"
      assert component.mpn == "some updated mpn"
      assert component.package == "some updated package"
      assert component.part == "some updated part"
      assert component.price == 456.7
      assert component.stock == 43
    end

    test "update_component/2 with invalid data returns error changeset" do
      component = component_fixture()
      assert {:error, %Ecto.Changeset{}} = JLC.update_component(component, @invalid_attrs)
      assert component == JLC.get_component!(component.id)
    end

    test "delete_component/1 deletes the component" do
      component = component_fixture()
      assert {:ok, %Component{}} = JLC.delete_component(component)
      assert_raise Ecto.NoResultsError, fn -> JLC.get_component!(component.id) end
    end

    test "change_component/1 returns a component changeset" do
      component = component_fixture()
      assert %Ecto.Changeset{} = JLC.change_component(component)
    end
  end
end
