#
# Main script
#
# Author: Marc Plouhinec
#

# Alibaba Cloud provider (source: https://github.com/terraform-providers/terraform-provider-alicloud)
provider "alicloud" {}

# VPC and VSwitch
resource "alicloud_vpc" "ctracker_vpc" {
  name = "chopstick-tracker-vpc"
  cidr_block = "192.168.0.0/16"
}
data "alicloud_zones" "az" {
  network_type = "Vpc"
  available_disk_category = "cloud_ssd"
  available_instance_type = "${var.instance_type}"
}
resource "alicloud_vswitch" "ctracker_vswitch" {
  name = "chopstick-tracker-vswitch"
  availability_zone = "${data.alicloud_zones.az.zones.0.id}"
  cidr_block = "192.168.0.0/24"
  vpc_id = "${alicloud_vpc.ctracker_vpc.id}"
}

# Security groups and rules
resource "alicloud_security_group" "ctracker_security_group" {
  name = "chopstick-tracker-security-group"
  vpc_id = "${alicloud_vpc.ctracker_vpc.id}"
}
resource "alicloud_security_group_rule" "accept_22_rule" {
  type = "ingress"
  ip_protocol = "tcp"
  nic_type = "intranet"
  policy = "accept"
  port_range = "22/22"
  priority = 1
  security_group_id = "${alicloud_security_group.ctracker_security_group.id}"
  cidr_ip = "0.0.0.0/0"
}

# Pay-as-you-go ECS instance
data "alicloud_images" "ubuntu_images" {
  owners = "system"
  name_regex = "ubuntu_18[a-zA-Z0-9_]+64"
  most_recent = true
}
resource "alicloud_instance" "ctracker_ecs" {
  instance_name = "chopstick-tracker-ecs"
  description = "Chopstick tracker."

  host_name = "chopstick-tracker-ecs"
  password = "${var.ecs_root_password}"

  image_id = "${data.alicloud_images.ubuntu_images.images.0.id}"
  instance_type = "${var.instance_type}"
  system_disk_category = "cloud_ssd"
  system_disk_size = 40

  internet_max_bandwidth_out = 1

  vswitch_id = "${alicloud_vswitch.ctracker_vswitch.id}"
  security_groups = [
    "${alicloud_security_group.ctracker_security_group.id}"
  ]

  provisioner "remote-exec" {
    connection {
      host = "${alicloud_instance.ctracker_ecs.public_ip}"
      user = "root"
      password = "${var.ecs_root_password}"
    }
    inline = [
      "mkdir -p /root/projects/chopsticks-tracker/data",
      "mkdir -p /root/projects/chopsticks-tracker/src"
    ]
  }

  provisioner "file" {
    connection {
      host = "${alicloud_instance.ctracker_ecs.public_ip}"
      user = "root"
      password = "${var.ecs_root_password}"
    }
    source = "${path.cwd}/../data"
    destination = "/root/projects/chopsticks-tracker/data/"
  }

  provisioner "file" {
    connection {
      host = "${alicloud_instance.ctracker_ecs.public_ip}"
      user = "root"
      password = "${var.ecs_root_password}"
    }
    source = "${path.cwd}/../src"
    destination = "/root/projects/chopsticks-tracker/src/"
  }

  provisioner "file" {
    connection {
      host = "${alicloud_instance.ctracker_ecs.public_ip}"
      user = "root"
      password = "${var.ecs_root_password}"
    }
    source = "${path.cwd}/../CMakeLists.txt"
    destination = "/root/projects/chopsticks-tracker/CMakeLists.txt"
  }

  provisioner "file" {
    connection {
      host = "${alicloud_instance.ctracker_ecs.public_ip}"
      user = "root"
      password = "${var.ecs_root_password}"
    }
    source = "${path.cwd}/../config.ini"
    destination = "/root/projects/chopsticks-tracker/config.ini"
  }

  provisioner "remote-exec" {
    connection {
      host = "${alicloud_instance.ctracker_ecs.public_ip}"
      user = "root"
      password = "${var.ecs_root_password}"
    }
    script = "install.sh"
  }
}